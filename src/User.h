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

    double timeslot;

    // Stats
    long  numberRBs;
    long  numServed;
    long  servedBytes;

    //Signals
    simsignal_t throughput_s;
    simsignal_t responseTime_s;
    simsignal_t CQI_s;
    simsignal_t numberRBs_s;
    simsignal_t served_s;

    virtual simsignal_t createDynamicSignal(std::string prefix, std::string templateName);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handleFrame(Frame* f);
    virtual void sendCQI();
    virtual void finish();

};

#endif
