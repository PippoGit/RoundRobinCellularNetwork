#include "User.h"

Define_Module(User);

int User::NEXT_USER_ID;

void User::initialize()
{
    waitMessage=new cMessage("waitMessage");
    scheduleAt(simTime(), waitMessage);
    userID = NEXT_USER_ID++;

    //Statistics

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
        //emit Signal

    }


}

void User::handleTimer()
{
    createNewPacket();
    cRNG *seed = getRNG(SEED_INTERARRIVAL);
    simtime_t lambda = par("lambda");
    interArrivalTime = omnetpp::exponential(seed, lambda);
    EV << " la lambda:    " << lambda;
    this->scheduleAt(simTime() + interArrivalTime, waitMessage);

}

void User::handleFrame(Frame* f)
{
    EV << "[USER] I have received a frame... Here is the content:" << endl;
    for(int i =0; i<FRAME_SIZE; i++)
    {
        if(f->getRBFrame(i).getRecipient()==userID)
        {
            int numFragments = f->getRBFrame(i).getNumFragments();
            EV << "[USER] There are " << numFragments << " fragments" << endl;
            for(int j = 0; j < numFragments; j++)
            {
                EV << "   ID PKT: " << f->getRBFrame(i).getFragment(j).id << endl;
                EV << "   PKT SIZE: " <<  f->getRBFrame(i).getFragment(j).packetSize << endl;
                EV << "   FRG SIZE: " <<  f->getRBFrame(i).getFragment(j).fragmentSize << endl;
            }
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
    packet->setReceiverID(omnetpp::intuniform(getRNG(SEED_RECIPIENT), MIN_USERS, numUsers - 1)); // la INT UNIFORM prende anche gli estremi!

    send(packet, "out");
}
