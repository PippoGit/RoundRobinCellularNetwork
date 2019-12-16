#ifndef USERINFORMATION_H_
#define USERINFORMATION_H_

#include <omnetpp.h>
#include <vector>
#include "ResourceBlock.h"
#include "constants.h"

class UserInformation {
private:
    // int id;
    omnetpp::cQueue FIFOQueue;
    int CQI;
    int numPendingPackets;

public: // too much stuff
    std::vector<ResourceBlock>::iterator lastRB;
    double remainingBytes;


public:
    UserInformation();
    virtual ~UserInformation();
    virtual void generateCQI(omnetpp::cRNG*seedCQI);

    virtual int              CQIToBytes();
    virtual omnetpp::cQueue* getQueue();

    virtual void incrementNumPendingPackets();
    virtual void setNumPendingPackets(int val);
    virtual void initNumPendingPackets();
    virtual int  getNumPendingPackets();
};

#endif /* USERINFORMATION_H_ */
