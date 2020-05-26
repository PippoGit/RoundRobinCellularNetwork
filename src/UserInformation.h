#ifndef USERINFORMATION_H_
#define USERINFORMATION_H_

#include <omnetpp.h>
#include <vector>
#include "ResourceBlock.h"
#include "constants.h"

class UserInformation {
private:
    int id;
    omnetpp::cQueue FIFOQueue;

    int CQI;
    int numPendingPackets;
    
    int  servedBytes;
    bool served;

    omnetpp::simsignal_t nq_s;


public:
    UserInformation(int id);
    virtual ~UserInformation();

    virtual int getId();
    virtual int              CQIToBytes();
    virtual omnetpp::cQueue* getQueue();
    virtual omnetpp::simsignal_t getNqSignal();
    virtual void setCQI(int cqi);
    virtual int  getCQI();


    virtual void serveUser();
    virtual bool isServed();
    virtual void shouldBeServed();

    virtual void incrementNumPendingPackets();
    virtual void setNumPendingPackets(int val);
    virtual void initNumPendingPackets();
    virtual int  getNumPendingPackets();

    virtual void incrementServedBytes(int bytes);
    virtual int  getServedBytes();

    virtual void setNqSignal(omnetpp::simsignal_t nq_s);
};

#endif /* USERINFORMATION_H_ */
