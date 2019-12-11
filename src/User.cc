#include "User.h"

Define_Module(User);

int User::NEXT_USER_ID;

void User::initialize()
{
    waitMessage=new cMessage("waitMessage");
    scheduleAt(simTime(), waitMessage);
    userID = NEXT_USER_ID++;
}

void User::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()){
        handleTimer();
    }
    else
    {
        Frame *f = check_and_cast<Frame*>(msg);
        handleFrame(f);
    }


}

void User::handleTimer()
{
    createNewPacket();
    cRNG *seed = getRNG(SEED_INTERARRIVAL);
    interArrivalTime = omnetpp::exponential(seed, SimTime(0.25));
    this->scheduleAt(simTime() + interArrivalTime, waitMessage);

}

void User::handleFrame(Frame* f)
{
    EV << "[USER] I have received a frame... Here is the content:" << endl;
    for(int i =0; i<FRAME_SIZE; i++)
    {
        if(f->getRBFrame(i).getRecipient()==userID)
        {
            EV << "[USER] There are " << f->getRBFrame(i).getNumPackets() << " packets for me" << endl;
            for(int j = 0; j < f->getRBFrame(i).getNumPackets(); j++)
                EV << " RB for me: SENDER: " << f->getRBFrame(i).getPacket(j)->getSenderID() << endl;
        }
    }
    delete(f);
}

void User::createNewPacket(){
    Packet *packet = new Packet();

    packet->setSenderID(this->userID);

    //for assumption: packet dimension between 1 and 75 bytes (La intuniform prende ENTRAMBI gli estremi)
    packet->setServiceDemand(omnetpp::intuniform(getRNG(SEED_SERVICE_DEMAND), MIN_SERVICE_DEMAND, MAX_SERVICE_DEMAND)); //F: secondo me dovrebbe essere un double

    int numUsers = this->getParentModule()->par("nUsers");
    packet->setReceiverID(omnetpp::intuniform(getRNG(SEED_RECIPIENT), MIN_USERS, numUsers));

    send(packet, "out");
}
