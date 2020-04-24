#pragma once
#include "WinETWSensor.h"
#include <map>
#include <mutex>

typedef std::map<std::wstring, UINT64> DomainCounterMap;
typedef std::map<ULONG, UINT64> ProcessCounterMap;
struct DnsStats
{
    DomainCounterMap DomainCounter;
    ProcessCounterMap ProcessCounter;
    UINT64 TotalQueries;
};

// Use WinETW API to collect DNS-Client Events
class DnsSensor : public WinETWSensor
{
public:
    DnsSensor();
    virtual std::wstring getInfo();
private:
    DnsStats _Stats;
    std::mutex _Lock;
protected:
    virtual void eventReceived(PEVENT_RECORD evt);
};
