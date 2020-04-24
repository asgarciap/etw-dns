#include "WinETWSensor.h"
#include <evntcons.h>

#pragma comment(lib, "tdh.lib")

WinETWSensor::WinETWSensor(LPWSTR name, GUID guid)
{
    _Name = name;
    _ProviderGuid = guid;
    _SessionTraceHandle = 0;
    _SessionProperties = NULL;
    _TraceOn = false;
}

WinETWSensor::~WinETWSensor()
{
    if (_SessionTraceHandle)
    {
        if (_TraceOn)
        {
            DWORD status = EnableTraceEx2(
                _SessionTraceHandle,
                (LPCGUID)&_ProviderGuid,
                EVENT_CONTROL_CODE_DISABLE_PROVIDER,
                TRACE_LEVEL_INFORMATION,
                0,
                0,
                0,
                NULL
            );
        }

        DWORD status = ControlTrace(_SessionTraceHandle, LOGSESSION_NAME, _SessionProperties, EVENT_TRACE_CONTROL_STOP);

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"ControlTrace(stop) failed with %lu\n", status);
        }
    }

    if (_SessionProperties)
    {
        free(_SessionProperties);
        _SessionProperties = NULL;
    }
}

bool WinETWSensor::init()
{
    ULONG status = ERROR_SUCCESS;
    ULONG BufferSize = 0;

    // Allocate memory for the session properties. The memory must
    // be large enough to include the log file name and session name,
    // which get appended to the end of the session properties structure.

    BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(LOGSESSION_NAME);
    _SessionProperties = (EVENT_TRACE_PROPERTIES*)malloc(BufferSize);
    if (NULL == _SessionProperties)
    {
        wprintf(L"Unable to allocate %d bytes for properties structure.\n", BufferSize);
        return false;
    }

    // Set the session properties. You only append the log file name
    // to the properties structure; the StartTrace function appends
    // the session name for you.

    ZeroMemory(_SessionProperties, BufferSize);
    _SessionProperties->Wnode.BufferSize = BufferSize;
    _SessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    _SessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
    _SessionProperties->Wnode.Guid = WINT_ETW_SESSION_GUID;
    _SessionProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    _SessionProperties->MaximumFileSize = 1;  // 1 MB
    _SessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

    // Create the trace session.

    status = StartTrace((PTRACEHANDLE)&_SessionTraceHandle, LOGSESSION_NAME, _SessionProperties);
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"StartTrace() failed with %lu\n", status);
        return false;
    }
    wprintf(L"StartTrace() successed\n");

    // Enable the providers that you want to log events to your session.

    status = EnableTraceEx2(
        _SessionTraceHandle,
        (LPCGUID)&_ProviderGuid,
        EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        TRACE_LEVEL_INFORMATION,
        0,
        0,
        0,
        NULL
    );

    if (ERROR_SUCCESS != status)
    {
        wprintf(L"EnableTrace() failed with %lu\n", status);
        _TraceOn = false;
        return false;
    }
    wprintf(L"EnableTrace() successed\n");
    _TraceOn = true;
    return true;
}

void WinETWSensor::runSensor() 
{
    if (_TraceOn)
    {
        TRACEHANDLE hTrace = 0;
        ULONG status = ERROR_SUCCESS;
        EVENT_TRACE_LOGFILE trace;
        TRACE_LOGFILE_HEADER* pHeader = &trace.LogfileHeader;

        // Identify the log file from which you want to consume events
        // and the callbacks used to process the events and buffers.

        ZeroMemory(&trace, sizeof(EVENT_TRACE_LOGFILE));
        trace.LoggerName = (LPWSTR)LOGSESSION_NAME;
        trace.EventRecordCallback = (PEVENT_RECORD_CALLBACK)(WinETWSensor::ProcessEvent);
        trace.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_REAL_TIME;
        trace.Context = this;

        hTrace = OpenTrace(&trace);
        if (INVALID_PROCESSTRACE_HANDLE == hTrace)
        {
            wprintf(L"OpenTrace failed with %lu\n", GetLastError());
            goto cleanup;
        }

        // Use pHeader to access all fields prior to LoggerName.
        // Adjust pHeader based on the pointer size to access
        // all fields after LogFileName. This is required only if
        // you are consuming events on an architecture that is 
        // different from architecture used to write the events.

        if (pHeader->PointerSize != sizeof(PVOID))
        {
            pHeader = (PTRACE_LOGFILE_HEADER)((PUCHAR)pHeader +
                2 * (pHeader->PointerSize - sizeof(PVOID)));
        }

        status = ProcessTrace(&hTrace, 1, 0, 0);
        if (status != ERROR_SUCCESS && status != ERROR_CANCELLED)
        {
            wprintf(L"ProcessTrace failed with %lu\n", status);
            goto cleanup;
        }

        cleanup:
            if (INVALID_PROCESSTRACE_HANDLE != hTrace)
            {
                status = CloseTrace(hTrace);
            }
    }
}

void WinETWSensor::stopSensor()
{
    DWORD status = ERROR_SUCCESS;
    wprintf(L"Stopping %s Sensor. Terminating Trace Session.\n",_Name.data());
    if (_SessionTraceHandle)
    {
        TRACEHANDLE SessionHandle = _SessionTraceHandle;
        _SessionTraceHandle = 0;
        status = EnableTraceEx2(
            SessionHandle,
            (LPCGUID)&_ProviderGuid,
            EVENT_CONTROL_CODE_DISABLE_PROVIDER,
            TRACE_LEVEL_INFORMATION,
            0,
            0,
            0,
            NULL
        );

        status = ControlTrace(SessionHandle, LOGSESSION_NAME, _SessionProperties, EVENT_TRACE_CONTROL_STOP);

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"ControlTrace(stop) failed with %lu\n", status);
        }
    }

    if (_SessionProperties)
    {
        EVENT_TRACE_PROPERTIES* pSessionProperties = _SessionProperties;
        _SessionProperties = NULL;
        free(pSessionProperties);
        pSessionProperties = NULL;
    }
}

VOID WINAPI WinETWSensor::ProcessEvent(PEVENT_RECORD pEvent)
{
    DWORD status = ERROR_SUCCESS;
    PTRACE_EVENT_INFO pInfo = NULL;
    LPWSTR pwsEventGuid = NULL;
    ULONGLONG TimeStamp = 0;
    ULONGLONG Nanoseconds = 0;
    SYSTEMTIME st;
    SYSTEMTIME stLocal;
    FILETIME ft;

    // Skips the event if it is the event trace header. Log files contain this event
    // but real-time sessions do not. The event contains the same information as 
    // the EVENT_TRACE_LOGFILE.LogfileHeader member that you can access when you open 
    // the trace. 

    if (IsEqualGUID(pEvent->EventHeader.ProviderId, EventTraceGuid) &&
        pEvent->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO)
    {
        ; // Skip this event.
    }
    else
    {
        // Process the event. The pEvent->UserData member is a pointer to 
        // the event specific data, if it exists.

        status = WinETWSensor::GetEventInformation(pEvent, pInfo);

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"GetEventInformation failed with %lu\n", status);
            goto cleanup;
        }

        // Determine whether the event is defined by a MOF class, in an
        // instrumentation manifest, or a WPP template; to use TDH to decode
        // the event, it must be defined by one of these three sources.

        if (DecodingSourceWbem == pInfo->DecodingSource)  // MOF class
        {
            HRESULT hr = StringFromCLSID(pInfo->EventGuid, &pwsEventGuid);

            if (FAILED(hr))
            {
                wprintf(L"StringFromCLSID failed with 0x%x\n", hr);
                status = hr;
                goto cleanup;
            }

            wprintf(L"\nEvent GUID: %s\n", pwsEventGuid);
            CoTaskMemFree(pwsEventGuid);
            pwsEventGuid = NULL;

            wprintf(L"Event version: %d\n", pEvent->EventHeader.EventDescriptor.Version);
            wprintf(L"Event type: %d\n", pEvent->EventHeader.EventDescriptor.Opcode);
        }
        else if (DecodingSourceXMLFile == pInfo->DecodingSource) // Instrumentation manifest
        {
            wprintf(L"Event ID: %d\n", pInfo->EventDescriptor.Id);
        }
        else // Not handling the WPP case
        {
            goto cleanup;
        }

        // Print the time stamp for when the event occurred.

        ft.dwHighDateTime = pEvent->EventHeader.TimeStamp.HighPart;
        ft.dwLowDateTime = pEvent->EventHeader.TimeStamp.LowPart;

        FileTimeToSystemTime(&ft, &st);
        SystemTimeToTzSpecificLocalTime(NULL, &st, &stLocal);

        TimeStamp = pEvent->EventHeader.TimeStamp.QuadPart;
        Nanoseconds = (TimeStamp % 10000000) * 100;

        wprintf(L"%02d/%02d/%02d %02d:%02d:%02d.%I64u\n",
            stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute, stLocal.wSecond, Nanoseconds);

        if (EVENT_HEADER_FLAG_STRING_ONLY == (pEvent->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
        {
            // The Event doesnt have any property. Print it for debugging purposes.
            wprintf(L"%s\n", (LPWSTR)pEvent->UserData);
        }
        else
        {
            // Call the eventReceived function to inform that an event have been received.
            WinETWSensor *obj = (WinETWSensor*) pEvent->UserContext;
            obj->eventReceived(pEvent);
            goto cleanup;
        }
    }

cleanup:

    if (pInfo)
    {
        free(pInfo);
    }
}

BOOL WinETWSensor::getPropertyValue(PEVENT_RECORD evt, LPWSTR prop, PBYTE *pData)
{
    PROPERTY_DATA_DESCRIPTOR DataDescriptors;
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    *pData = NULL;
    DWORD status = ERROR_SUCCESS;

    ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));
    DataDescriptors.PropertyName = (ULONGLONG) (prop);
    DataDescriptors.ArrayIndex = 0;
    DescriptorsCount = 1;

    status = TdhGetPropertySize(evt, 0, NULL, DescriptorsCount, &DataDescriptors, &PropertySize);

    if (ERROR_SUCCESS != status)
    {
        wprintf(L"TdhGetPropertySize failed with %lu\n", status);
        return FALSE;
    }

    *pData = (PBYTE)malloc(PropertySize);

    if (NULL == *pData)
    {
        wprintf(L"Failed to allocate memory for property data\n");
        status = ERROR_OUTOFMEMORY;
        return FALSE;
    }

    status = TdhGetProperty(evt, 0, NULL, DescriptorsCount, &DataDescriptors, PropertySize, *pData);

    if (ERROR_SUCCESS != status)
    {
        wprintf(L"TdhGetProperty failed with %lu\n", status);
        if (*pData)
        {
            free(*pData);
            *pData = NULL;
        }
        return FALSE;
    }

    return TRUE;
}

// Get the metadata for the event.
DWORD WinETWSensor::GetEventInformation(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO& pInfo)
{
    DWORD status = ERROR_SUCCESS;
    DWORD BufferSize = 0;

    // Retrieve the required buffer size for the event metadata.

    status = TdhGetEventInformation(pEvent, 0, NULL, pInfo, &BufferSize);

    if (ERROR_INSUFFICIENT_BUFFER == status)
    {
        pInfo = (TRACE_EVENT_INFO*)malloc(BufferSize);
        if (pInfo == NULL)
        {
            wprintf(L"Failed to allocate memory for event info (size=%lu).\n", BufferSize);
            status = ERROR_OUTOFMEMORY;
            goto cleanup;
        }

        // Retrieve the event metadata.

        status = TdhGetEventInformation(pEvent, 0, NULL, pInfo, &BufferSize);
    }

    if (ERROR_SUCCESS != status)
    {
        wprintf(L"TdhGetEventInformation failed with 0x%x.\n", status);
    }

cleanup:

    return status;
}

