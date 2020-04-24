#pragma once
#include <string>
#include <mutex>

using namespace std;

//Base class to represent a sensor from Agentx
//A sensor is an object that collects information
//from the current machine where the agent is running.
class AgentSensor
{
protected:
    std::wstring _Name;
    std::thread _ThreadId;
protected:
    virtual bool init() = 0;
    virtual void runSensor() = 0;
    virtual void stopSensor() = 0;
public:
    AgentSensor();
    virtual bool start();
    virtual void stop();
    bool isRunning();
    virtual std::string getInfo() = 0;
};
