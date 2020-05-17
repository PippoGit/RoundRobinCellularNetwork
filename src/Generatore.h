
#ifndef __ROUNDROBINCELLULARNETWORK_GENERATORE_H_
#define __ROUNDROBINCELLULARNETWORK_GENERATORE_H_

#include <omnetpp.h>
#include "Packet_m.h"
#include "constants.h"

using namespace omnetpp;


class Generatore : public cSimpleModule
{
  private:
    static int NEXT_USER_ID;
    int userId;
    cMessage *pt;

  protected:
    virtual void initialize();
    virtual void generatePacket();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif
