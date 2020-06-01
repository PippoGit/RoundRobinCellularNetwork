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

    struct rb_inspection_result_t {
        int  last_seen;
        long served_bytes;
        long number_rbs;
        long number_pkts;

        rb_inspection_result_t():last_seen(-1), served_bytes(0), number_rbs(0), number_pkts(0) {}
    };

    //Signals
    simsignal_t throughput_s;
    simsignal_t responseTime_s;
    simsignal_t waitingTime_s;
    simsignal_t serviceTime_s;
    simsignal_t CQI_s;
    simsignal_t numberRBs_s;
    simsignal_t numberPkts_s;
    simsignal_t served_s;

    // private methods
    virtual simsignal_t createDynamicSignal(std::string prefix, std::string templateName);
    virtual void        inspectResourceBlock(const ResourceBlock &rb, rb_inspection_result_t &res);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handleFrame(Frame* f);
    virtual void sendCQI();
    virtual void finish();

};

#endif