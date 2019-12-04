#ifndef __ROUNDROBINCELLULARNETWORK_ANTENNA_H_
#define __ROUNDROBINCELLULARNETWORK_ANTENNA_H_

#include <omnetpp.h>
#include "UserInformation.h"

using namespace omnetpp;

class Antenna : public cSimpleModule
{
  private:
    UserInformation *users;
    cMessage *timer;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void roundRobin();
};

#endif
