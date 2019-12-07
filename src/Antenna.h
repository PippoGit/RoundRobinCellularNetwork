#ifndef __ROUNDROBINCELLULARNETWORK_ANTENNA_H_
#define __ROUNDROBINCELLULARNETWORK_ANTENNA_H_

#include <omnetpp.h>
#include "UserInformation.h"
#include "ResourceBlock.h"
#include "Packet_m.h"
#include "Frame_m.h"
#include "constants.h"

using namespace omnetpp;

class Antenna : public cSimpleModule
{
  private:
    int NUM_USERS;
    std::vector<UserInformation> users;
    cMessage *timer;

    // stuff for roundrobin
    std::vector<UserInformation>::iterator currentUser;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handlePacket(Packet *p);
    virtual void downlinkPropagation();

    virtual void updateCQIs();
    virtual void roundrobin();
    virtual void broadcastFrame(Frame *f);
    virtual void fillFrameWithCurrentUser(std::vector<ResourceBlock>::iterator &from, std::vector<ResourceBlock>::iterator to);

};

#endif
