#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing antenna..." << endl;
    NUM_USERS = this->getParentModule()->par("nUsers");
    timer = new cMessage("timer");

    EV_DEBUG << "[ANTENNA-INITIALIZE] Building UserInformation data structure" << endl;
    users.reserve(NUM_USERS);
    for(int i=0; i < NUM_USERS; i++)
        users.push_back(UserInformation());

    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing first iterator" << endl;
    currentUser = users.end()-1; // this will make the first call to roundrobin() to set currentUser to begin()

    // schedule first iteration of RR algorithm
    scheduleAt(simTime(), timer);
}

void Antenna::updateCQIs()
{
    for(std::vector<UserInformation>::iterator it = users.begin(); it != users.end(); ++it)
    {
        cRNG *seedUser = getRNG(SEED_CQI);
        it->generateCQI(seedUser);
    }
}


void Antenna::roundrobin()
{
    currentUser = (currentUser == users.end()-1)?users.begin():currentUser+1;
    EV_DEBUG << "[ROUND_ROBIN] it's the turn of " << (users.begin()-currentUser) << endl;
}


Frame* Antenna::vectorToFrame(std::vector<ResourceBlock> &v)
{
    Frame *f = new Frame();
    for(auto it=v.begin(); it != v.end(); ++it)
    {
        f->setRBFrame(it - v.begin(), *it);
    }
    return f;
}



Frame* Antenna::duplicateFrame(Frame *f)
{
    Frame *copy = f->dup();

    for(int i=0; i<FRAME_SIZE; ++i)
    {
        ResourceBlock rbi(f->getRBFrame(i));
        copy->setRBFrame(i, rbi);
    }
    return copy;
}

void Antenna::destroyFrame(Frame *f)
{
    for(int i=0; i<FRAME_SIZE; ++i)
        f->getRBFrame(i).deletePackets();
    delete f;
}


void Antenna::broadcastFrame(Frame *f)
{
    for(int i=0; i<NUM_USERS; ++i)
    {
        Frame *copy = duplicateFrame(f);
        send(copy, "out", i);
    }
    destroyFrame(f);
}


void Antenna::fillFrameWithCurrentUser(std::vector<ResourceBlock>::iterator &from, std::vector<ResourceBlock>::iterator to)
{
    cQueue *queue = currentUser->getQueue();

    while(!(queue->isEmpty() || from == to))
    {
        EV_DEBUG << "[DOWNLINK] Non empty queue" << endl;
        Packet *p          = check_and_cast<Packet*>(queue->front());
        std::vector<UserInformation>::iterator recipient = users.begin() + p->getReceiverID();
        double packetSize  = p->getServiceDemand();
        int    rCQI        = currentUser->CQIToBytes();
        int    requiredRBs = ceil(packetSize/rCQI);

        if(packetSize <= recipient->remainingBytes)
        {
            // the packet can be put inside last RB
            EV_DEBUG << "[DOWNLINK] This packet fits the remaining bytes of previous RB" << endl;
            recipient->remainingBytes -= packetSize;
            from->setRecipient(p->getReceiverID());
            from->appendPacket(p);
        }
        else if (requiredRBs <= to - from)
        {
            // the packet can be put in the next rbs
            EV_DEBUG << "[DOWNLINK] This packet can be put in the frame somewhere" << endl;

            // TODO: Questa cosa ANDREBBE CAMBIATA! Si potrebbe trovare un modo
            // migliore di gestire questa cosa. Invece che copiare n volte sempre tutti
            // i pacchetti, che non è la soluzione più elegante...
            ResourceBlock b(p->getSenderID(), p->getReceiverID());
            for(auto it = from; it != from + requiredRBs; ++it)
                *it = ResourceBlock(b);

            recipient->remainingBytes -= packetSize;

            // increment pointers
            from += requiredRBs;
        }
        else break; // not enough space (this is the most aweful piece of code ever)

        // If i get to this point it means the packet was put somewhere in the
        // frame so we can pop it from its queue.
        queue->remove(p);
    }
}


void Antenna::downlinkPropagation()
{
    int numIterations = 0;
    std::vector<UserInformation>::iterator firstUser = currentUser;
    std::vector<ResourceBlock> frame(FRAME_SIZE);
    std::vector<ResourceBlock>::iterator currentRB = frame.begin();

    EV_DEBUG << "[DOWNLINK] Updating CQI..." <<endl;

    // 1) Get updated CQIs
    updateCQIs();

    // 2) Round-robin over  allthe users...
    do
    {
        // Select next queue
        roundrobin();

        EV_DEBUG << "[DOWNLINK] It's the turn of: " << currentUser - users.begin() << endl;

        // Fill the frame with current user's queue and update currentRB index
        fillFrameWithCurrentUser(currentRB, frame.end());

        numIterations += (currentUser == firstUser);
    } while(currentRB != frame.end() && numIterations < 2);

    // 3) send the frame to all the users
    broadcastFrame(vectorToFrame(frame));
    EV_DEBUG << "[DOWNLINK] Broadcast propagation of the frame" << endl;

    // Schedule next iteration
    simtime_t timeslot_dt = par("timeslot");
    scheduleAt(simTime() + timeslot_dt, timer);
}


void Antenna::handlePacket(Packet *p)
{
    int userId = p->getSenderID();
    EV_DEBUG << "[UPLINK] Received a new packet to be put into the queue of " << userId << endl;

    users[userId].getQueue()->insert(p);
}


void Antenna::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
        downlinkPropagation();
    else
        handlePacket(check_and_cast<Packet*>(msg));
}
