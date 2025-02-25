#ifndef __ROUNDROBINCELLULARNETWORK_ANTENNA_H_
#define __ROUNDROBINCELLULARNETWORK_ANTENNA_H_

#include <omnetpp.h>
#include "UserInformation.h"
#include "ResourceBlock.h"

#include "Packet_m.h"
#include "Frame_m.h"
#include "PacketCQI_m.h"

#include "constants.h"

using namespace omnetpp;

class Antenna : public cSimpleModule
{
  public:

    struct packet_info_t
    {
        int  recipient;
        bool served;
        int  size;
    };

  private:
    int NUM_USERS;
    std::vector<UserInformation> users;
    cMessage *timer;
    double timeslot;

    // stuff for roundrobin
    std::vector<UserInformation>::iterator currentUser;
    Frame *frame;

    // Information that are useful for performance evaluation
    std::map<long, Antenna::packet_info_t> packetsInformation;

    int  numServedUsersPerTimeslot;
    long numSentBytesPerTimeslot;
    long numPacketsPerTimeslot;
    int  numInServicePkts;

    // Global Stats Signal
    simsignal_t numServedUser_s;
    simsignal_t throughputAntenna_s;
    simsignal_t numberRBAntenna_s;
    simsignal_t numberPktAntenna_s;
    simsignal_t responseTimeAntenna_s;
    simsignal_t numberPktInService_s;

    std::vector<simtime_t> in_frame_arrivalTime;

  protected:
    virtual void finish();

    virtual void initialize();
    virtual void flushQueues();

    virtual void handleTimer(cMessage *msg);
    virtual void handleMessage(cMessage *msg);
    virtual void handlePacket(Packet *packet);
    virtual void handleCQI(PacketCQI *notification);

    virtual void downlinkPropagation();
    virtual void createFrame();

    virtual void   initRoundInformation();
    virtual void   roundrobin();
    virtual void   broadcastFrame(Frame *f);
    virtual void   fillFrameWithCurrentUser(std::vector<ResourceBlock>::iterator &from, std::vector<ResourceBlock>::iterator to);
    virtual Frame* vectorToFrame(std::vector<ResourceBlock> &v);

    virtual simsignal_t createDynamicSignal(std::string prefix, std::string templateName, int userID);
};

#endif
