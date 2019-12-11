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
    static int NEXT_USER_ID;
    simtime_t interArrivalTime; // exponential
    cMessage* waitMessage;
    int userID;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void createNewPacket();
    virtual void handleTimer();
    virtual void handleFrame(Frame* f);
};

#endif
