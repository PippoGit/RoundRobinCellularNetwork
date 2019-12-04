#ifndef __ROUNDROBINCELLULARNETWORK_ANTENNA_H_
#define __ROUNDROBINCELLULARNETWORK_ANTENNA_H_

#include <omnetpp.h>
#include "UserInformation.h"
#include "ResourceBlock.h"
#include "Packet_m.h"

using namespace omnetpp;

class Antenna : public cSimpleModule
{
  private:
    const int FRAME_SIZE = 25;
    int NUM_USERS;
    UserInformation *users;
    cMessage *timer;

    // stuff for roundrobin
    int currentUser;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handlePacket(Packet *p);
    virtual void downlinkPropagation();

    virtual void             updateCQIs();
    virtual UserInformation* roundrobin();
    virtual void             broadcastFrame(Frame *f);
    virtual bool             frameFull();

};

#endif
