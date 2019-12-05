#ifndef USERINFORMATION_H_
#define USERINFORMATION_H_

#include <omnetpp.h>
#include <vector>

class UserInformation {
private:
    omnetpp::cQueue *FIFOQueue;
    int CQI;


public:
    UserInformation();
    virtual ~UserInformation();
    virtual void generateCQI();

    virtual int     CQIToBytes();
    virtual omnetpp::cQueue* getQueue();
};

#endif /* USERINFORMATION_H_ */
