#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    NUM_USERS = 10; // this->getParentModule()->par("numUsers");
    users = new UserInformation[NUM_USERS];
    timer = new cMessage("timer");
    currentUser = 0;
}

void Antenna::updateCQIs()
{
    for(UserInformation *u : users)
        u->generateCQI();
}


int Antenna::CQIToBytes(int cqi)
{
    int bytes[] = {3, 3, 6, 11, 15, 20, 25, 36, 39, 50, 63, 72, 80, 93, 93};
    return bytes[cqi-1];
}


UserInformation* Antenna::roundrobin()
{
    return users[(currentUser++ + 1)%NUM_USERS];
}


bool Antenna::frameFull()
{
    return false;
}

void Antenna::downlinkPropagation()
{
    double next_timeslot = simTime().dbl() + this->par("timeslot");

    // 1) Get updated CQIs
    updateCQIs();

    // 2) I guess i need to check all the queue?
    do
    {
        UserInformation *u = roundrobin();

    } while(!frameFull());

    // That's it...
    scheduleAt(next_timeslot, timer);
}


void Antenna::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) // TEMPORARY ASSUMPTION: The frame is READY before the next
        downlinkPropagation();

}
