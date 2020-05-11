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
public:
    virtual ~LoggerChannel() = default;
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
    virtual ~FileLoggerChannel();
protected:
    virtual void logMessage(const char* str);
};

class ConsoleLoggerChannel : public LoggerChannel
{
public:
    virtual ~ConsoleLoggerChannel() = default;
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
    virtual ~Logger();
    void setLogLevel(LogLevel level);
    void addLoggerChannel(LoggerChannel* logger);
    void log(LogLevel level, const char* fmt, ...);
};

