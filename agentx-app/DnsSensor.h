#pragma once
#include "WinETWSensor.h"
#include <map>
#include <mutex>

typedef std::map<std::wstring, UINT64> WStringCounterMap;
struct DnsStats
{
    WStringCounterMap DomainCounter;
    WStringCounterMap ProcessCounter;
    UINT64 TotalQueries;
};

// Use WinETW API to collect DNS-Client Events
class DnsSensor : public WinETWSensor
{
public:
    DnsSensor();
    virtual ~DnsSensor() = default;
    virtual std::wstring getInfo();
private:
    DnsStats _Stats;
    std::mutex _Lock;
protected:
    virtual void eventReceived(PEVENT_RECORD evt);
private:
    std::wstring getProcessName(ULONG pid);
};
