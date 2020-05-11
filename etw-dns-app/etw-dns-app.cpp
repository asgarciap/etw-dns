// etw-dns-app.cpp : This file contains the 'main' function. Program execution begins and ends there.

//Turns the DEFINE_GUID for EventTraceGuid into a const.
#define INITGUID

#include <signal.h>
#include "DnsSensor.h"
#include "Logger.h"

bool g_Run = true;
Logger g_systemLog;

void SignalHandler(int signal);
typedef void (*SignalHandlerPointer)(int);

int wmain(void)
{
    g_systemLog.addLoggerChannel(new ConsoleLoggerChannel());
    g_systemLog.addLoggerChannel(new FileLoggerChannel("app.log"));
    g_systemLog.setLogLevel(DBG);

    g_systemLog.log(INF, "Starting etw-dns Application");

    Logger auditLog;
    auditLog.setLogLevel(ADT);
    auditLog.addLoggerChannel(new FileLoggerChannel("audit.log"));

    DnsSensor sensor;
    sensor.setAuditLogger(&auditLog);
    sensor.setSystemLogger(&g_systemLog);

    bool started = sensor.start();

    if(started)
    {
        SignalHandlerPointer previousHandler;
        previousHandler = signal(SIGINT, SignalHandler);
        previousHandler = signal(SIGTERM, SignalHandler);

        time_t lastUpdate = time(NULL);
        int printInfoFreq = 5;
        while (g_Run)
        {
            if ((time(NULL) - lastUpdate) > printInfoFreq)
            {
                //TODO sent info to a separate file
                wprintf(L"%s", sensor.getInfo().data());
                lastUpdate = time(NULL);
            }
            Sleep(1000);
        }
        sensor.stop();

        //print the last report before exit
        wprintf(L"%s", sensor.getInfo().data());
    }

    g_systemLog.log(INF, "etw-dns Application stopped");
    return 0;
}

void SignalHandler(int signal)
{
    g_systemLog.log(INF, "Signal received. Terminating process");
    g_Run = FALSE;
}
