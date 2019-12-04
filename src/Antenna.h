#ifndef __ROUNDROBINCELLULARNETWORK_ANTENNA_H_
#define __ROUNDROBINCELLULARNETWORK_ANTENNA_H_

#include <omnetpp.h>
#include "UserInformation.h"

using namespace omnetpp;

class Antenna : public cSimpleModule
{
  private:
    int NUM_USERS;
    UserInformation *users;
    cMessage *timer;

    // stuff for roundrobin
    int currentUser;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void downlinkPropagation();

    virtual int              CQIToBytes(int cqi);
    virtual void             updateCQIs();
    virtual UserInformation* roundrobin();

};

#endif
