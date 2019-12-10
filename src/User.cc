#include "User.h"

Define_Module(User);

void User::initialize()
{
    waitMessage=new cMessage("waitMessage");
    scheduleAt(simTime(), waitMessage);
}

void User::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()){
        createNewPacket();
        cRNG *seed = getRNG(SEED_INTERARRIVAL);
        interArrivalTime = omnetpp::exponential(seed, SimTime(0.25));
        this->scheduleAt(simTime() + interArrivalTime, waitMessage);
    }
    else
    {
        Frame *f = check_and_cast<Frame*>(msg);
        EV << "[USER] I have received a frame... Here is the content:" << endl;

        /*
        for(int i =0; i<FRAME_SIZE; i++) {
            EV << "     " << i << ") => Recipient: " << f->getRBFrame(i).getRecipient() << ", Sender: " << f->getRBFrame(i).getSender() << endl;
            // EV << " extracting something from RB: SENDER: " << f->getRBFrame(i).getPacket(0)->getSenderID() << endl;
        }
        */

    }


}

void User:: createNewPacket(){
    Packet *packet = new Packet();


    packet->setSenderID(this->userID);

    //for assumption: packet dimension between 1 and 75 bytes (La intuniform prende ENTRAMBI gli estremi)
    packet->setServiceDemand(omnetpp::intuniform(getRNG(SEED_SERVICE_DEMAND), MIN_SERVICE_DEMAND, MAX_SERVICE_DEMAND)); //F: secondo me dovrebbe essere un double

    int numUsers = this->getParentModule()->par("nUsers");
    packet->setReceiverID(omnetpp::intuniform(getRNG(SEED_RECIPIENT), MIN_USERS, numUsers));

    send(packet, "out");
}
