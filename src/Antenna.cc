#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    EV_DEBUG << "[ANTENNA] Initializing antenna..." << endl;
    NUM_USERS = 10; // this->getParentModule()->par("numUsers");
    timer = new cMessage("timer");

    EV_DEBUG << "[ANTENNA-INITIALIZE] Building UserInformation datastructure" << endl;
    users.reserve(NUM_USERS);
    for(int i=0; i < NUM_USERS; i++)
        users.push_back(UserInformation());

    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing first iterator" << endl;
    currentUser = users.end(); // this will make the first call to roundrobin to set currentUser to begin()

    EV_DEBUG << "[ANTENNA-INITIALIZE] Creating a random bunch of packets..." << endl;
    // Just fill the queues with random stuff....
    for(std::vector<UserInformation>::iterator it = users.begin(); it != users.end(); it++)
    {
        int i = uniform(0, NUM_USERS);
        EV_DEBUG << "[ANTENNA-INITIALIZE] Allocating " << i << " packets for: " << it->getUserId() << endl;

        while(i++<10) {
            std::string name = "testPkt-" + std::to_string(it->getUserId()) + "-" + std::to_string(i);
            Packet *pkt = new Packet(name.c_str());
            pkt->setSenderID(it->getUserId());
            pkt->setReceiverID(omnetpp::intuniform(getRNG(1), 0, NUM_USERS)); // just a test... it doesn't need to be accurate
            pkt->setServiceDemand(omnetpp::intuniform(getRNG(1), 0, NUM_USERS));
            it->getQueue()->insert(pkt);
        }
    }

    // test
    downlinkPropagation();
}

void Antenna::updateCQIs()
{
    for(std::vector<UserInformation>::iterator it = users.begin(); it != users.end(); it++)
    {
        cRNG *seedUser = getRNG(SEED_CQI + it->getUserId());
        it->generateCQI(seedUser);
    }
}


void Antenna::roundrobin()
{
    currentUser = (currentUser == users.end())?users.begin():currentUser+1;
    EV_DEBUG << "[ROUND_ROBIN] it's the turn of " << currentUser->getUserId();
}


void Antenna::broadcastFrame(Frame *f)
{
    // for simplicity just send it to every users and then each user will check
    // if there is something for them.
    for(int i=0; i<NUM_USERS; i++)
    {
        Frame *copy = f->dup();
        send(copy, "out", i);
    }
    delete f;
}


int Antenna::fillFrameWithCurrentUser(std::vector<ResourceBlock>::iterator from, std::vector<ResourceBlock>::iterator to)
{
    cQueue *queue = currentUser->getQueue();

    while(!queue->isEmpty() || from == to)
    {
        Packet *p          = check_and_cast<Packet*>(queue->front());
        double packetSize  = p->getServiceDemand();
        int    rCQI        = currentUser->CQIToBytes();
        int    requiredRBs = ceil(packetSize/rCQI);

        EV << "   packet content: " << p->getName();

        // check if something is wrong with the size of the next packet
        if(currentUser->remainingBytes <= packetSize)
        {
            // the packet can be put inside last RB
        }
        else if (requiredRBs <= to - from)
        {
            // the packet can be put in the next rbs
        }
        else break; // not enough space (this is the most aweful piece of code ever)

        // if i'm here i need to do another iteration (increment of the iterators)
        from++;
    }

    // return final position of the frame
    return (from - to); // THIS IS WRONG! It is not actually doing what is supposed to do.
}


void Antenna::downlinkPropagation()
{
    bool frameFull = false;
    double next_timeslot = simTime().dbl() + (double) this->par("timeslot");
    int currentRB = 0;
    std::vector<ResourceBlock> frame(FRAME_SIZE); // Frame *f = new Frame();

    // 1) Get updated CQIs
    updateCQIs();

    // 2) Round-robin over all the users...
    do
    {
        // Select next queue
        roundrobin();

        // Fill the frame with current user's queue and update currentRB index
        currentRB = fillFrameWithCurrentUser(frame.begin()+currentRB, frame.end());
    } while(currentRB < FRAME_SIZE);

    // 3) send the frame to all the users
    // broadcastFrame(f);

    // Schedule next iteration
    scheduleAt(next_timeslot, timer);
}


void Antenna::handlePacket(Packet *p)
{

}


void Antenna::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) // TEMPORARY ASSUMPTION: The current frame is READY before the next
        downlinkPropagation();
    else
        handlePacket(check_and_cast<Packet*>(msg));
}
