#ifndef USERINFORMATION_H_
#define USERINFORMATION_H_

#include <omnetpp.h>
#include <vector>
#include "ResourceBlock.h"
#include "constants.h"
#include "PacketTimer_m.h"

class UserInformation {
private:
    int id;
    omnetpp::cQueue FIFOQueue;
    int CQI;
    int numPendingPackets;
    int numberRBs;
    
    int  servedBytes;
    bool served;

    PacketTimer *timer;


public:
    // too much stuff
    // std::vector<ResourceBlock>::iterator lastRB;
    // double remainingBytes;

    omnetpp::simsignal_t throughput_s;
    omnetpp::simsignal_t responseTime_s;
    omnetpp::simsignal_t CQI_s;
    omnetpp::simsignal_t numberRBs_s;

public:
    UserInformation(int id);
    virtual ~UserInformation();
    virtual void generateCQI(omnetpp::cRNG*RNG, bool isBinomial);
    virtual int getId();

    virtual int              CQIToBytes();
    virtual omnetpp::cQueue* getQueue();

    virtual void incrementNumPendingPackets();
    virtual void setNumPendingPackets(int val);
    virtual void initNumPendingPackets();
    virtual int  getNumPendingPackets();

    virtual void serveUser();
    virtual bool isServed();
    virtual void shouldBeServed();

    virtual void setCQI(int cqi);
    virtual int getCQI();
    virtual void incrementServedBytes(int bytes);
    virtual int  getServedBytes();
    virtual void incrementNumberRBs();
    virtual int  getNumberRBs();
    virtual void setNumberRBs(int n);

    virtual void setTimer(PacketTimer *t);
    PacketTimer* getTimer() { return timer; };
};

#endif /* USERINFORMATION_H_ */
