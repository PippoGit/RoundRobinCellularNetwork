#ifndef __ROUNDROBINCELLULARNETWORK_USER_H_
#define __ROUNDROBINCELLULARNETWORK_USER_H_

#include <omnetpp.h>

using namespace omnetpp;

class User : public cSimpleModule
{
  private:


  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
