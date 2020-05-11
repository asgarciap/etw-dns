#pragma once
#include "Logger.h"
#include <string>
#include <mutex>

//Base class to represent a sensor from Agentx
//A sensor is an object that collects information
//from the current machine where the agent is running.
class AgentSensor
{
protected:
    std::wstring _Name;
    std::thread _ThreadId;
    Logger* _SystemLogger;
    Logger* _AuditLogger;
protected:
    virtual bool init() = 0;
    virtual void runSensor() = 0;
    virtual void stopSensor() = 0;
public:
    AgentSensor();
    virtual ~AgentSensor() = default;
    virtual bool start();
    virtual void stop();
    bool isRunning();
    void setSystemLogger(Logger* logger);
    void setAuditLogger(Logger* logger);
    Logger* getSystemLogger();
    Logger* getAuditLogger();
    virtual std::wstring getInfo() = 0;
};
