#pragma once
#include "WinETWSensor.h"

// Use WinETW API to collect DNS-Client Events
class DnsSensor : public WinETWSensor
{
public:
    DnsSensor();
    virtual std::string getInfo();
protected:
    virtual void eventReceived(PEVENT_RECORD evt);
};

