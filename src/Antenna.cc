#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    NUM_USERS = 10; // this->getParentModule()->par("numUsers");
    timer = new cMessage("timer");

    users.reserve(NUM_USERS);
    currentUser = users.end(); // this will make the first call to roundrobin to set currentUser to begin()

    // Just fill the queues with random stuff....
    for(UserInformation u:users)
    {
        int i = uniform(0, 100);
        while(i++<100) {
            std::string name = "testPckt-" + i;
            cMessage *packet = new cMessage(name.c_str());
            cMsgPar  *size = new cMsgPar("size");
            size->setDoubleValue(uniform(0, 100)); //bytes...
            packet->addPar(size);
            u.getQueue()->insert(packet);
        }
    }
}

void Antenna::updateCQIs()
{
    for(UserInformation u : users)
        u.generateCQI();
}


void Antenna::roundrobin()
{
    currentUser = (currentUser == users.end())?users.begin():currentUser+1;
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
        cMessage *p = check_and_cast<cMessage*>(queue->front());
        double packetSize = (double) p->par("size");
        int requiredRBs = ceil(packetSize/currentUser->CQIToBytes());

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
    return (from - to);
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
