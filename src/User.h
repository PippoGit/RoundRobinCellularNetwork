#ifndef __ROUNDROBINCELLULARNETWORK_USER_H_
#define __ROUNDROBINCELLULARNETWORK_USER_H_

#include <omnetpp.h>
#include "constants.h"
#include "Frame_m.h"
#include "PacketCQI_m.h"

using namespace omnetpp;

class User : public cSimpleModule
{
  private:
    static int NEXT_USER_ID;
    int userID;
    cMessage *pt;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handleFrame(Frame* f);
    virtual void sendCQI();
    virtual void finish();
};

#endif
