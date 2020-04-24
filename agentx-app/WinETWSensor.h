#pragma once
#include "AgentSensor.h"
#include <windows.h>
#include <evntrace.h>
#include <tdh.h>

//This class encapsulates a Sensor that collects
//all of its data from the Win ETW API.
//It starts an event trace session and register
//the specified provider to receive real time events

//TODO Best option is to use a unique global trace session 

// TODO generate next GUID dynamically
static const GUID WINT_ETW_SESSION_GUID =
{ 0xae44cb98, 0xbd11, 0x4069, { 0x80, 0x93, 0x77, 0xe, 0xc9, 0x25, 0x8a, 0x10 } };

#define LOGSESSION_NAME L"Agentx Trace"

class WinETWSensor : public AgentSensor
{
protected:
    GUID _ProviderGuid;
    TRACEHANDLE _SessionTraceHandle;
    EVENT_TRACE_PROPERTIES* _SessionProperties;
    bool _TraceOn;
protected:
    virtual bool init();
    virtual void runSensor();
    virtual void stopSensor();
    virtual void eventReceived(PEVENT_RECORD evt) = 0;
    BOOL getPropertyValue(PEVENT_RECORD evt, LPWSTR prop, PBYTE *pData);
public:
    WinETWSensor(LPWSTR name, GUID guid);
    ~WinETWSensor();
    static VOID WINAPI ProcessEvent(PEVENT_RECORD pEvent);
    DWORD GetEventInformation(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO& pInfo);
};
