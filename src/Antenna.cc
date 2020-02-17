#include "Antenna.h"

Define_Module(Antenna);

simsignal_t Antenna::createDynamicSignal(std::string prefix, int userId, std::string templateName)
{
    simsignal_t signal;
    std::string signal_name;
    std::stringstream sstream;

    sstream << prefix << "-" << userId;
    signal_name = sstream.str();
    
    signal = registerSignal(signal_name.c_str());

    cProperty *statisticTemplate = getProperties()->get("statisticTemplate", templateName.c_str());
    getEnvir()->addResultRecorders(this, signal, signal_name.c_str(), statisticTemplate);
    return signal;
}

void Antenna::initialize()
{
    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing antenna..." << endl;
    NUM_USERS = this->getParentModule()->par("nUsers");
    timer = new cMessage("roundrobin");
    timer->setKind(MSG_RR_TIMER);

    EV_DEBUG << "[ANTENNA-INITIALIZE] Building UserInformation data structure" << endl;
    users.reserve(NUM_USERS);
    for(int i=0; i < NUM_USERS; i++)
    {
        UserInformation u(i);
        u.throughput_s   = createDynamicSignal("tptUser", i, "tptUserTemplate");
        u.responseTime_s = createDynamicSignal("responseTime", i, "responseTimeUserTemplate");
        
        // set the timer
        PacketTimer *pt = new PacketTimer();
        pt->setKind(MSG_PKT_TIMER);
        pt->setUserId(i);
        u.setTimer(pt);
        scheduleAt(simTime() + /*(i+1)*0.001 */ exponential((simtime_t) par("lambda"), RNG_INTERARRIVAL),pt);
        users.push_back(u);
    }


    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing first iterator" << endl;
    currentUser = users.end()-1; // this will make the first call to roundrobin() to set currentUser to begin()

    //signals
    responseTimeGlobal_s  = registerSignal("responseTimeGlobal");
    throughput_s          = registerSignal("throughput");
    numServedUser_s       = registerSignal("NumServedUser");
    numberRB_s			  = registerSignal("numberRB");

    // schedule first iteration of RR algorithm
    frame = nullptr;
    numSentBytesPerTimeslot   = 0;
    numServedUsersPerTimeslot = 0;
    scheduleAt(simTime(), timer);
}


void Antenna::initUsersInformation()
{
    // THIS METHOD SHOULD RESET ALL THE INFORMATION THAT ARE VALID FOR A TIMESLOT
    // THROUGHPUT INFORMATION AND CQIs
    bool isBinomial = par("isBinomial");
    double successProbGroup1 = par("successProbGroup1");
    double successProbGroup2 = par("successProbGroup2");
    double successProbGroup3 = par("successProbGroup3");
    for(std::vector<UserInformation>::iterator it = users.begin(); it != users.end(); ++it)
    {
        double p = (it->getId()<3)? successProbGroup1: (it->getId()<7)? successProbGroup2: successProbGroup3;
        int cqi = (isBinomial)?binomial(BINOMIAL_N, p):intuniform(MIN_CQI, MAX_CQI);
        EV << "User: " << it->getId() << " - p: " << p << " - cqi: "<<cqi<<endl;
        it->setCQI(cqi);
        it->shouldBeServed();
    }
    numServedUsersPerTimeslot = 0;
    numSentBytesPerTimeslot   = 0;
}


void Antenna::roundrobin()
{
    currentUser = (currentUser == users.end()-1)?users.begin():currentUser+1;
    EV_DEBUG << "[ROUND_ROBIN] it's the turn of " << currentUser->getId() << endl;
}


Frame* Antenna::vectorToFrame(std::vector<ResourceBlock> &v)
{
    Frame *f = new Frame();
    int c=0;
    for(auto it=v.begin(); it != v.end(); ++it)
    {
        f->setRBFrame(it - v.begin(), *it);
        c += (it->getRecipient() > 0);
    }
    f->setAllocatedRBs(c);
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
    int currentUserId = currentUser->getId();

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

    do
    {
        EV_DEBUG << "[CREATE_FRAME] Round Robin Starting Up..." <<endl;
        roundrobin();

        if(currentUser == lastUser)
           break;

        // Fill the frame with current user's queue and update currentRB index
        fillFrameWithCurrentUser(currentRB, vframe.end());
    } while(currentRB != vframe.end());

    // 3) send the frame to all the users DURING NEXT TIMESLOT!
    this->frame = vectorToFrame(vframe);

    // Schedule next iteration
    simtime_t timeslot_dt = par("timeslot");
    scheduleAt(simTime() + timeslot_dt, timer);
}


void Antenna::handlePacket(int userId)
{
    EV_DEBUG << "[UPLINK] Create a new packet to be put into the queue of " << userId << endl;
    Packet *packet = new Packet();

    if (packet != nullptr)
        EV_DEBUG << "[DEBUG_ISH?] the packet is generated at " << packet << endl;

    EV_DEBUG << "[UPLINK] Adding some random service demand for the packet" << endl;
    packet->setServiceDemand(intuniform(MIN_SERVICE_DEMAND, MAX_SERVICE_DEMAND, RNG_SERVICE_DEMAND));
    EV_DEBUG << "[UPLINK] Setting the recipient for the packet (" << userId <<")" << endl;
    packet->setReceiverID(userId);

    EV_DEBUG << "[UPLINK] Create a data structure for the new packet with ID " << packet->getId() << endl;
    // this is a new packet! so we are going to keep its info somewhere!
    Antenna::packet_info_t i;
    i.arrivalTime = simTime();
    i.served = false;
    i.recipient = userId;

    EV_DEBUG << "[UPLINK] Inserting packet with ID " << packet->getId() << " in the packetsInformation hashmap" << endl;
    packetsInformation.insert(std::pair<long, Antenna::packet_info_t>(packet->getId(), i));

    EV_DEBUG << "[UPLINK] Inserting packet with ID " << packet->getId() << " in the Queue for the user " << userId << endl;
    EV_DEBUG << "[UPLINK] Getting the queue... " << endl;
    cQueue *q = users[userId].getQueue();
    EV_DEBUG << "[UPLINK] Queue: " << q << endl;

    EV_DEBUG << "[UPLINK] Inserting packet " << packet << endl;
    q->insert(packet);
    EV_DEBUG << "[UPLINK] INSERTED! "<< endl;


    // SCHEDULE NEXT PACKET
    EV_DEBUG << "[UPLINK] Scheduling next packet for User-" << userId << endl;
    simtime_t lambda = par("lambda");
    EV_DEBUG << " LAMBDA: " << lambda << " DOUBLE: " << lambda.dbl() << endl;
    scheduleAt(simTime() + /*(userId+1)*0.001 */ exponential(lambda, RNG_INTERARRIVAL), users[userId].getTimer());
    EV_DEBUG << "[UPLINK] Done!" << endl;
}


void Antenna::downlinkPropagation()
{
    if(frame == nullptr) return; // first iteration...

    // Update the info about the packet being in the frame
    for(long id : pendingPackets)
    {
        Antenna::packet_info_t info = packetsInformation.at(id);
        info.propagationTime = simTime();

        // emit responsetime...
        // TEST !!!!!!
        if (simTime() > getSimulation()->getWarmupPeriod()) {
            emit(users[info.recipient].responseTime_s, info.propagationTime - info.arrivalTime);
            users[info.recipient].incrementServedBytes(info.size);
            emit(responseTimeGlobal_s,  info.propagationTime - info.arrivalTime);
        }
        ////////

        packetsInformation.erase(id); // remove the packet from the hash table
    }
    emit(numberRB_s, frame->getAllocatedRBs());
    broadcastFrame(frame);
    EV_DEBUG << "[DOWNLINK] Broadcast propagation of the frame" << endl;

    if (simTime() > getSimulation()->getWarmupPeriod()) {
        EV_DEBUG << "[ANTENNA] Emitting signals for global statistics " << endl;
        emit(throughput_s,    numSentBytesPerTimeslot);   //Tpt defined as bytes sent per timeslot
        emit(numServedUser_s, numServedUsersPerTimeslot); // Tpt defined as num of served users per timeslot
    }

    // Emit statitics per user
    EV_DEBUG << "[ANTENNA] Emitting signals for user's statistics " << endl;
    for(auto it=users.begin(); it!=users.end(); ++it)
    {
        if (simTime() > getSimulation()->getWarmupPeriod())
            emit(it->throughput_s, it->getServedBytes());
    }

    pendingPackets.clear(); // clear the pending packets data structure...
}


void Antenna::handleMessage(cMessage *msg)
{
    EV_DEBUG << "[ANTENNA] New message to be handled!" << endl;

    if(msg->getKind() == MSG_RR_TIMER)
    {
        EV_DEBUG << " [ANTENA RR] Start propagation of the previous frame..." << endl;
        downlinkPropagation();
        EV_DEBUG << " [*** ANTENA RR ***] Create new frame..." << endl;
        createFrame();
        EV_DEBUG << " [*** ANTENA RR ***] DONE!" << endl;
    }
    else if(msg->getKind() == MSG_PKT_TIMER)
    {
        PacketTimer *t = check_and_cast<PacketTimer*>(msg);
        EV_DEBUG << "[ANTENNA PKT-TMR] A new packet should be generate: " << t->getUserId() << endl;
        handlePacket(t->getUserId());
    }
}


void Antenna::finish() {
    // drop(timer);

    cancelAndDelete(timer);
    for(auto it=users.begin(); it!=users.end(); ++it)
        cancelAndDelete(it->getTimer());

    cancelAndDelete(frame);
}

Antenna::~Antenna()
{
    // delete []tptUser_s;

    // delete timer;
    // for(auto it=users.begin(); it!=users.end(); ++it)
        // delete it->getTimer();
}

