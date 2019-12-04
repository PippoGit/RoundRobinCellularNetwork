#ifndef __ROUNDROBINCELLULARNETWORK_USER_H_
#define __ROUNDROBINCELLULARNETWORK_USER_H_

#include <omnetpp.h>
#include "Packet.msg"

using namespace omnetpp;

class User : public cSimpleModule
{
  private:
    simtime_t interArrivalTime; // exponential
    cMessage* waitMessage;
    Packet* packet;
    int userID;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handleTimer();
};

#endif
