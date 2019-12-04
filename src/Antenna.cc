#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    int numUsers = 10; // this->getParentModule()->par("numUsers");
    users = new UserInformation[numUsers];
    timer = new cMessage("timer");
}

void Antenna::roundRobin() {

    // next round...
    this->scheduleAt(simTime() + timeslot, timer);
}

void Antenna::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
        roundRobin();
}
