#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing antenna..." << endl;
    NUM_USERS = this->getParentModule()->par("nUsers");
    timer = new cMessage("roundrobin");
    timer->setKind(MSG_RR_TIMER);

    EV_DEBUG << "[ANTENNA-INITIALIZE] Building UserInformation data structure" << endl;
    users.reserve(NUM_USERS);
    for(int i=0; i < NUM_USERS; i++)
        users.push_back(UserInformation());

    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing first iterator" << endl;
    currentUser = users.end()-1; // this will make the first call to roundrobin() to set currentUser to begin()

    //signals
    responseTime_s  = registerSignal("responseTime");
    throughput_s    = registerSignal("throughput");
    numServedUser_s = registerSignal("NumServedUser");

    for(int i=0; i < NUM_USERS; i++)
    {
        char signalName[30];
        sprintf(signalName, "tptUser-%d", i);
        simsignal_t signal = registerSignal(signalName);
        cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "tptUserTemplate");
        getEnvir()->addResultRecorders(this, signal, signalName, statisticTemplate);
        users[i].throughput_s = signal;

        char tmrName[30];
        sprintf(tmrName, "pkt-%d", i);
        cMessage *tmr = new cMessage(tmrName);
        tmr->setKind(MSG_PKT_TIMER);
        users[i].setTimer(tmr);
        scheduleAt(simTime(), tmr);
    }

    // schedule first iteration of RR algorithm
    frame = nullptr;
    scheduleAt(simTime(), timer);
    numSentBytesPerTimeslot   = 0;
    numServedUsersPerTimeslot = 0;
}


void Antenna::initUsersInformation()
{
    // THIS METHOD SHOULD RESET ALL THE INFORMATION THAT ARE VALID FOR A TIMESLOT
    // THROUGHPUT INFORMATION AND CQIs
    bool isBinomial = par("isBinomial");

    for(std::vector<UserInformation>::iterator it = users.begin(); it != users.end(); ++it)
    {
        int cqi = (isBinomial)?binomial(0, 0):intuniform(MIN_CQI, MAX_CQI);
        it->setCQI(cqi);
        it->shouldBeServed();
    }
    numServedUsersPerTimeslot = 0;
    numSentBytesPerTimeslot   = 0;
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
    // It's the turn of CurrentUser, let's take its queue and info...
    cQueue *queue     = currentUser->getQueue();
    int uCQI          = currentUser->CQIToBytes();
    int currentUserId = (currentUser - users.begin());

    int    remainingRBs        = (to-from);
    double totalRemainingBytes = (remainingRBs * uCQI);

    EV_DEBUG << "[CREATE_FRAME RR] SET UP " << endl;
    EV_DEBUG << "    USER CQI        : " << uCQI << endl;
    EV_DEBUG << "    REMAINING RBs   : " << remainingRBs << endl;
    EV_DEBUG << "    REMAINING BYTES : " << totalRemainingBytes << endl;


    while(!(queue->isEmpty() || from == to))
    {
        EV_DEBUG << "[CREATE_FRAME RR] Non empty queue" << endl;

        Packet *p = check_and_cast<Packet*>(queue->front());

        //IF IT'S THE FIRST TIME YOU CONSIDER THE PACKET, UPDATE ITS START-SERVICE-TIME VARIABLE
        if(!packetsInformation[p->getId()].served)
        {
            packetsInformation[p->getId()].servedTime = simTime();
            packetsInformation[p->getId()].served     = true;
        }

        double packetSize          = p->getServiceDemand(),
               residualPacketSize  = packetSize,
               residualRequiredRBs = packetSize/uCQI;


        EV_DEBUG << "[CREATE_FRAME RR] PACKET TO INSERT" << endl;
        EV_DEBUG << "   PACKET SIZE  : " << packetSize << endl;

        if(packetSize <= totalRemainingBytes)
        {
            // CurrentUser will now be "served"
            // The user is consider for service only if there is enough space for the
            // first packet in its queue
            if(!currentUser->isServed())
            {
                numServedUsersPerTimeslot++;
                currentUser->serveUser();

            }

            // If there is space, it means that i'm going to send that packet
            // SO the packet will become "pending"
            pendingPackets.push_back(p->getId());
            numSentBytesPerTimeslot += packetSize;
            currentUser->incrementNumPendingPackets();

            // WHEN A PACKET INSERTED IN FRAME, START-FRAME-TIME
            packetsInformation[p->getId()].frameTime = simTime();
            packetsInformation[p->getId()].size      = packetSize;

            // The packet will be put somewhere in the frame, so decrease the number
            // of bytes available in the frame (for this user)
            totalRemainingBytes -= packetSize;


            /////////////////////////////////////////////////////
            EV_DEBUG << "[CREATE_FRAME RR] Inserting " << packetSize << "Bytes at " << (FRAME_SIZE) - (to - from) << endl;
            EV_DEBUG << "    REQUIRED RBs:   " << residualRequiredRBs << endl;
            EV_DEBUG << "    REMAINING RBs:  " << (to - from) << endl;

            while(residualPacketSize > 0)
            {
                // if nobody wrote on this RB, i'll take it!
                // if it is NOT available, it is because it was already allocated
                // to currentUser in the previous iteration!
                if(from->isAvailable())
                    from->allocResourceBlock(currentUserId, uCQI);

                double fragmentSize = std::min(residualPacketSize, static_cast<double>(uCQI));

                EV_DEBUG << "Adding fragment:    " << endl;
                EV_DEBUG << "    FRAGMENT SIZE:  " << fragmentSize << endl;
                EV_DEBUG << "    RESIDUAL SIZE:  " << residualPacketSize << endl;
                EV_DEBUG << "    REMAINING RBs:  " << (to - from) << endl;
                EV_DEBUG << "    INDEX:          " << (FRAME_SIZE) - (to - from) << endl;

                from->appendFragment(p, fragmentSize);

                residualPacketSize -= fragmentSize;

                // if current RB is full, consider the next one
                if(from->isFull())
                    ++from;
            }
            /////////////////////////////////////////////////////
            queue->remove(p);
            delete p; // also delete the packet!
        }
        else break;
    }

    // If the last RB was not filled (AND the frame is not full)
    if(!(from == to || from->isAvailable()))
        ++from; // the next user should start at from+1
}


void Antenna::createFrame()
{
    std::vector<UserInformation>::iterator lastUser = currentUser;
    std::vector<ResourceBlock> vframe(FRAME_SIZE);
    std::vector<ResourceBlock>::iterator currentRB = vframe.begin();

    EV_DEBUG << "[CREATE_FRAME] Updating CQI..." <<endl;

    // 1) Get updated CQIs
    initUsersInformation();

    roundrobin();
        do
        {
            EV_DEBUG << "[CREATE_FRAME] Round Robin Starting Up..." <<endl;
            // Fill the frame with current user's queue and update currentRB index
            fillFrameWithCurrentUser(currentRB, vframe.end());
            roundrobin();
        } while(!(currentRB == vframe.end() || currentUser == lastUser));

    // 3) send the frame to all the users DURING NEXT TIMESLOT!
    this->frame = vectorToFrame(vframe);

    // simtime_t meanResponseTime = (this->frame->getSumServiceTimes()+this->frame->getSumWaitingTimes()+timeslot)/this->frame->getNumPackets();

    // Schedule next iteration
    simtime_t timeslot_dt = par("timeslot");
    scheduleAt(simTime() + timeslot_dt, timer);
}


void Antenna::handlePacket(int userId)
{
    EV_DEBUG << "[UPLINK] Create a new packet to be put into the queue of " << userId << endl;

    Packet *packet = new Packet();
    packet->setServiceDemand(intuniform(MIN_SERVICE_DEMAND, MAX_SERVICE_DEMAND, RNG_SERVICE_DEMAND));
    packet->setReceiverID(userId);

    // this is a new packet! so we are going to keep its info somewhere!
    Antenna::packet_info_t i;
    i.arrivalTime = simTime();
    i.served = false;
    packetsInformation.insert(std::pair<long, Antenna::packet_info_t>(packet->getId(), i));
    users[userId].getQueue()->insert(packet);

    // SCHEDULE NEXT PACKET
    simtime_t lambda = par("lambda");
    scheduleAt(simTime() + exponential(lambda, RNG_INTERARRIVAL), users[userId].getTimer());
}


void Antenna::downlinkPropagation()
{
    if(frame == nullptr) return; // first iteration...

    // Update the info about the packet being in the frame
    for(long id : pendingPackets)
    {
        Antenna::packet_info_t info = packetsInformation.at(id);
        info.propagationTime = simTime();

        emit(responseTime_s, info.propagationTime.dbl() - info.arrivalTime.dbl());

        // Increment bytes sent for this user...
        users[info.sender].incrementServedBytes(info.size);

        packetsInformation.erase(id); // remove the packet from the hash table
    }

    broadcastFrame(frame);
    EV << "[DOWNLINK] Broadcast propagation of the frame" << endl;

    EV << "[ANTENNA] Emitting signals for global statistics " << endl;
    emit(throughput_s,    numSentBytesPerTimeslot);   //Tpt defined as bytes sent per timeslot
    emit(numServedUser_s, numServedUsersPerTimeslot); // Tpt defined as num of served users per timeslot

    // Emit statitics per user
    EV << "[ANTENNA] Emitting signals for user's statistics " << endl;
    for(auto it=users.begin(); it!=users.end(); ++it)
    {
       emit(it->throughput_s, it->getServedBytes());
    }

    pendingPackets.clear(); // clear the pending packets data structure...
}


void Antenna::handleMessage(cMessage *msg)
{
    if(msg->getKind() == MSG_RR_TIMER)
    {
        downlinkPropagation();
        createFrame();
    }
    else if(msg->getKind() == MSG_PKT_TIMER)
    {
        int userId;
        EV_DEBUG << "[ANTENNA PKT-TMR] A new packet should be generate: " << msg->getName() << endl;
        sscanf(msg->getName(), "pkt-%d", &userId);
        handlePacket(userId);
    }
}


Antenna::~Antenna()
{
    // delete []tptUser_s;
    delete timer;
    for(auto it=users.begin(); it!=users.end(); ++it)
        delete it->getTimer();
}

