#pragma once
#include <vector>
#include <fstream>

//TODO move each class to a separate file to be more organized
typedef enum
{
    DBG,
    INF,
    ERR,
    ADT
} LogLevel;

class LoggerChannel
{
protected:
    virtual void logMessage(const char* str) = 0;
    friend class Logger;
};

class FileLoggerChannel : public LoggerChannel
{
private:
    std::ofstream _LogFile;
public:
    FileLoggerChannel(const char* logfile);
    ~FileLoggerChannel();
protected:
    virtual void logMessage(const char* str);
};

class ConsoleLoggerChannel : public LoggerChannel
{
protected:
    virtual void logMessage(const char* str);
};

// Base class to generate logs
class Logger
{
private:
    LogLevel _LogLevel;
    std::vector<LoggerChannel*> _LoggerChannels;
public:
    ~Logger();
    void setLogLevel(LogLevel level);
    void addLoggerChannel(LoggerChannel* logger);
    void log(LogLevel level, const char* fmt, ...);
};

