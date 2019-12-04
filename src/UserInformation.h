#ifndef USERINFORMATION_H_
#define USERINFORMATION_H_

#include <omnetpp.h>
#include <cqueue.h>
#include <crandom.h>
#include <vector>

class UserInformation {
private:
    cQueue *FIFOQueue;
    int CQI;


public:
    UserInformation();
    virtual ~UserInformation();
    virtual void generateCQI();

    virtual int     CQIToBytes();
    virtual cQueue* getQueue();
};

#endif /* USERINFORMATION_H_ */
