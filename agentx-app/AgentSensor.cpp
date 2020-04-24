#include "AgentSensor.h"
#include <thread>

AgentSensor::AgentSensor()
{
}

bool AgentSensor::start()
{
    if (!init())
    {
        return false;
    }
    _ThreadId = std::thread(&AgentSensor::runSensor, this);
    return true;
}

void AgentSensor::stop()
{
    stopSensor();
    _ThreadId.join();
}

bool AgentSensor::isRunning()
{
    return false;
}