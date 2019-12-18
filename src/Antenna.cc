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
    frame = nullptr;
    scheduleAt(simTime(), timer);
    numSentBytes=0;
    numServedUsers = 0;

    //signals
    responseTime_s=registerSignal("responseTime");
    throughput_s= registerSignal("throughput");
    NumServedUser_s=registerSignal("NumServedUser");

    // tptUsers_s = new simsignal_t[NUM_USERS];

    for(int i=0; i < NUM_USERS; i++)
    {
        char signalName[30];
        sprintf(signalName, "tptUser-%d", i);
        simsignal_t signal = registerSignal(signalName);
        cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "tptUserTemplate");
        getEnvir()->addResultRecorders(this, signal, signalName, statisticTemplate);
        users[i].throughput = signal;
    }
}


void Antenna::initUsersInformation()
{
    //THIS METHOD SHOULD RESET ALL THE INFORMATION THAT ARE VALID FOR A TIMESLOT
    // THROUGHPUT INFORMATION AND CQIs

    bool isBinomial=par("isBinomial");
    for(std::vector<UserInformation>::iterator it = users.begin(); it != users.end(); ++it)
    {
        // it->generateCQI(getRNG(RNG_CQI), isBinomial); OLD VERSION

        // VIRDIS-LIKE VERSION:
        int cqi = (isBinomial)?binomial(0, 0):intuniform(MIN_CQI, MAX_CQI);
        it->setCQI(cqi);
        it->shouldBeServed();

    }
    numServedUsers = 0;
    numSentBytes   = 0;
}


void Antenna::roundrobin()
{
    currentUser = (currentUser == users.end()-1)?users.begin():currentUser+1;
    EV_DEBUG << "[ROUND_ROBIN] it's the turn of " << (currentUser - users.begin()) << endl;
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


void Antenna::broadcastFrame(Frame *f)
{
    for(int i=0; i<NUM_USERS; ++i)
    {
        Frame *copy = f->dup();
        send(copy, "out", i);
    }
    delete f;
}


void Antenna::fillFrameWithCurrentUser(std::vector<ResourceBlock>::iterator &from, std::vector<ResourceBlock>::iterator to)
{
    cQueue *queue = currentUser->getQueue();

    while(!(queue->isEmpty() || from == to))
    {
        EV_DEBUG << "[CREATE_FRAME RR] Non empty queue" << endl;
        Packet *p = check_and_cast<Packet*>(queue->front());

        //IF IT'S THE FIRST TIME YOU CONSIDER THE PACKET, UPDATE ITS START-SERVICE-TIME VARIABLE
        if(!packetsInformation[p->getId()].served)
            {
            packetsInformation[p->getId()].servedTime = simTime();
            packetsInformation[p->getId()].served=true;
            }
        std::vector<UserInformation>::iterator recipient = users.begin() + p->getReceiverID();
        // (I have checked: believe it or not, this is the right recipient)

        double packetSize           = p->getServiceDemand();
        double residualPacketSize   = packetSize;
        int    rCQI                 = recipient->CQIToBytes(); // Is this wrong??
        double residualRequiredRBs  = packetSize/rCQI;
        int    remainingRBs         = (to-from);

        // IF THERE is ENOUGH SPACE FOR THE WHOLE PACKET!
        double totalRemainingBytes = (remainingRBs * rCQI) + (recipient->remainingBytes < rCQI)*recipient->remainingBytes;
        // I have to put the remainingBytes ONLY if that slot is half-full

        if(packetSize <= totalRemainingBytes)
        {

            if(!currentUser->isServed()) {
                numServedUsers++;
                currentUser->serveUser();
            }

            numSentBytes += packetSize;

            currentUser->incrementNumPendingPackets();
            // If there is space, it means that i'm going to put the packet somewhere!
            // SO the packet will become "pending"
            pendingPackets.push_back(p->getId());

            //WHEN A PACKET INSERTED IN FRAME, START-FRAME-TIME
            packetsInformation[p->getId()].frameTime = simTime();
            packetsInformation[p->getId()].size = packetSize;

            // 1) fill the lastRB
            if(recipient->remainingBytes > 0)
            {
                EV_DEBUG << "[CREATE_FRAME RR] Putting " << recipient->remainingBytes << " in last RB" << endl;
                EV_DEBUG << "     Inserting at index: " << (FRAME_SIZE) - (to - recipient->lastRB) << endl;

                if(recipient->lastRB == to) recipient->lastRB = from; // to is just another name for .end()

                double fragmentSize = std::min(packetSize, recipient->remainingBytes);

                residualPacketSize  -= fragmentSize;
                residualRequiredRBs  = residualPacketSize/rCQI;

                recipient->lastRB->setRecipient(p->getReceiverID());
                recipient->lastRB->appendFragment(p, fragmentSize);
                recipient->remainingBytes -= fragmentSize;
            }

            // 2) If there are still some bytes to write, put them at "from"
            if(residualPacketSize > 0)
            {
                EV_DEBUG << "[CREATE_FRAME RR] Putting remaining bytes... " << endl;
                EV_DEBUG << "    RESIDUAL SIZE:  " << residualPacketSize << endl;
                EV_DEBUG << "    REQUIRED RBs:   " << residualRequiredRBs << endl;
                EV_DEBUG << "    REMAINING:      " << (to - from) << endl;
                EV_DEBUG << "    INDEX:          " << (FRAME_SIZE) - (to - from) << endl;

                // if (recipient->lastRB == from) from++; // if lastRB was equal to from i have to move to the following RB

                double fragmentSize;
                while(residualPacketSize > 0)
                {
                    int currentIndex = FRAME_SIZE - (to - from);

                    EV_DEBUG << "    Inserting fragment at RB: " << currentIndex << endl;
                    fragmentSize = std::min(residualPacketSize, static_cast<double>(rCQI));
                    EV_DEBUG << "    The size for the fragment is: " << fragmentSize << endl;

                    from->setRecipient(p->getReceiverID());
                    from->setSender(p->getSenderID());
                    from->appendFragment(p, fragmentSize);

                    residualPacketSize -= fragmentSize;
                    ++from;
                }

                // Update lastRB for recipient
                recipient->lastRB = from-1;
                recipient->remainingBytes = rCQI - fragmentSize; // last fragment size
            }

            // 3) The packet was put somewhere...
            queue->remove(p);
            delete p; // also delete the packet!
        }
        else break;

    }
}


void Antenna::initUsersLastRBs(std::vector<ResourceBlock>::iterator end)
{
    for(auto it = users.begin(); it != users.end(); ++it)
        it->lastRB = end;
}


void Antenna::createFrame()
{
    int numIterations = 0;
    std::vector<UserInformation>::iterator firstUser = currentUser;
    std::vector<ResourceBlock> vframe(FRAME_SIZE);
    std::vector<ResourceBlock>::iterator currentRB = vframe.begin();

    initUsersLastRBs(vframe.end());

    EV_DEBUG << "[CREATE_FRAME] Updating CQI..." <<endl;

    // 1) Get updated CQIs
    initUsersInformation();

    // 2) Round-robin over  allthe users...
    do
    {
        // Select next queue
        roundrobin();

        // Fill the frame with current user's queue and update currentRB index
        fillFrameWithCurrentUser(currentRB, vframe.end());

        numIterations += (currentUser == firstUser);
    } while(currentRB != vframe.end() && numIterations < 2);

    // 3) send the frame to all the users DURING NEXT TIMESLOT!
    this->frame = vectorToFrame(vframe);

    // simtime_t meanResponseTime = (this->frame->getSumServiceTimes()+this->frame->getSumWaitingTimes()+timeslot)/this->frame->getNumPackets();


    // Schedule next iteration
    simtime_t timeslot_dt = par("timeslot");
    scheduleAt(simTime() + timeslot_dt, timer);
}


void Antenna::handlePacket(Packet *p)
{
    int userId = p->getSenderID();
    EV_DEBUG << "[UPLINK] Received a new packet to be put into the queue of " << userId << endl;

    // this is a new packet! so we are going to keep its info somewhere!
    Antenna::packet_info_t i;
    i.arrivalTime = simTime();
    i.served = false;
    i.sender = p->getSenderID();
    packetsInformation.insert(std::pair<long, Antenna::packet_info_t>(p->getId(), i));
    users[userId].getQueue()->insert(p);
}


void Antenna::downlinkPropagation()
{
    if(frame == nullptr) return; // first iteration...

    // Update the info about the packet being in the frame
    double tpt;
    for(long id : pendingPackets)
    {
        Antenna::packet_info_t info = packetsInformation.at(id);
        info.propagationTime = simTime();

        // ->
        //SIGNAL
                    //emit(waitTime_s,info.frameTime - info.arrivalTime);
                    // COMPUTE RESPONSE TIME
                     //simtime_t timeslot = par("timeslot");
                     // SIGNAL
                    //emit(responseTime_s,waitTime_s+timeslot);

        emit(responseTime_s, info.propagationTime.dbl() - info.arrivalTime.dbl());

        // Increment bytes sent for this user...
        users[info.sender].incrementServedBytes(info.size);

        packetsInformation.erase(id); // remove the packet from the hash table
    }

    broadcastFrame(frame);
    EV_DEBUG << "[DOWNLINK] Broadcast propagation of the frame" << endl;

    double timeslot = par("timeslot");
    emit(throughput_s, numServedUsers*(numSentBytes/timeslot));
    emit(NumServedUser_s,numServedUsers);

    // Emit throughput per user
    for(auto it=users.begin(); it!=users.end(); ++it)
    {
        emit(it->throughput, it->getServedBytes());
    }


    pendingPackets.clear(); // clear the pending packets data structure...
}


void Antenna::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        downlinkPropagation();
        createFrame();
    }
    else
        handlePacket(check_and_cast<Packet*>(msg));
}


Antenna::~Antenna()
{
    // delete []tptUser_s;
}

