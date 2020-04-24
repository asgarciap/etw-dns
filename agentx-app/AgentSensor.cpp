#include "AgentSensor.h"
#include <thread>
#include <locale>
#include <codecvt>

AgentSensor::AgentSensor()
{
    _SystemLogger = NULL;
    _AuditLogger = NULL;
}

void AgentSensor::setSystemLogger(Logger* logger)
{
    _SystemLogger = logger;
}

void AgentSensor::setAuditLogger(Logger* logger)
{
    _AuditLogger = logger;
}

Logger* AgentSensor::getSystemLogger()
{
    return _SystemLogger;
}

Logger* AgentSensor::getAuditLogger()
{
    return _AuditLogger;
}

bool AgentSensor::start()
{
    //convert _Name from wstring to string for logging purposes
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    std::string n = converter.to_bytes(_Name);

    _SystemLogger->log(INF, "Starting %s sensor", n.c_str());
    if (!init())
    {
        _SystemLogger->log(ERR, "Sensor initialization failed. Aborting start.");
        return false;
    }
    _ThreadId = std::thread(&AgentSensor::runSensor, this);
    _SystemLogger->log(INF, "Sensor %s started.", n.c_str());
    return true;
}

void AgentSensor::stop()
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    std::string n = converter.to_bytes(_Name);

    _SystemLogger->log(INF, "Stopping %s sensor", n.c_str());
    stopSensor();
    _ThreadId.join();
    _SystemLogger->log(INF, "Sensor %s stopped", n.c_str());
}

bool AgentSensor::isRunning()
{
    return false;
}
