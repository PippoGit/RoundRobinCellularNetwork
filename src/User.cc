#include "User.h"

Define_Module(User);

void User::initialize()
{
    waitMessage=new cMessage("waitMessage");

}

void User::handleMessage(cMessage *msg)
{
    if(msg->selfMessage()){
        createNewPacket();
        this->scheduleAt(simTime() + interArrivalTime, waitMessage);
    }


}

void User:: createNewPacket(){
    packet = new Packet();
    packet->senderID=this->userID;
    packet->creationTime=simTime();
    //for assumption: packet dimension between 1 and 75 bytes
    packet->serviceDemand=omnetpp::intuniform(rng, 1, 76);
    int numUsers=10; //this->getParentModule()->par("numUsers");
    packet->receiverID=omnetpp::intuniform(rng, 1, numUser+1);
}
