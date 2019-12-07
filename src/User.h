#ifndef __ROUNDROBINCELLULARNETWORK_USER_H_
#define __ROUNDROBINCELLULARNETWORK_USER_H_

#include <omnetpp.h>
#include "Packet_m.h"
#include "constants.h"
#include "Frame_m.h"

using namespace omnetpp;

class User : public cSimpleModule
{
  private:
    simtime_t interArrivalTime; // exponential
    cMessage* waitMessage;
    int userID;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void createNewPacket();
};

#endif
