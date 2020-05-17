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
    //signals (Per il momento lascio i segnali...)
    responseTimeGlobal_s  = registerSignal("responseTimeGlobal");
    throughput_s          = registerSignal("throughput");
    numServedUser_s       = registerSignal("NumServedUser");
    numberRB_s            = registerSignal("numberRB");


    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing antenna..." << endl;
    NUM_USERS = this->getParentModule()->par("nUsers");

    //if(NUM_USERS==0) return;
    timer = new cMessage("roundrobin");
    timer->setKind(MSG_RR_TIMER);

    EV_DEBUG << "[ANTENNA-INITIALIZE] Building UserInformation data structure" << endl;
    users.reserve(NUM_USERS);
    // simtime_t lambda = par("lambda");

    for(int i=0; i < NUM_USERS; i++)
    {
        UserInformation u(i);
        u.throughput_s   = createDynamicSignal("tptUser", i, "tptUserTemplate");
        u.responseTime_s = createDynamicSignal("responseTime", i, "responseTimeUserTemplate");
        u.CQI_s          = createDynamicSignal("CQI", i, "CQIUserTemplate");
        u.numberRBs_s    = createDynamicSignal("numberRBs", i, "numberRBsUserTemplate");
        u.served_s       = createDynamicSignal("servedUser", i, "servedUserTemplate");
        users.push_back(u);
    }

    EV_DEBUG << "[ANTENNA-INITIALIZE] Initializing first iterator" << endl;
    currentUser = users.end()-1; // this will make the first call to roundrobin() to set currentUser to begin()

    // schedule first iteration of RR algorithm
    frame = nullptr;
    initRoundInformation();
    scheduleAt(simTime(), timer);
}


void Antenna::initRoundInformation()
{
    // THIS METHOD SHOULD RESET ALL THE INFORMATION THAT ARE VALID FOR A TIMESLOT
    numServedUsersPerTimeslot = 0;
    numSentBytesPerTimeslot   = 0;
}


void Antenna::roundrobin()
{
    if(NUM_USERS > 0) {
        currentUser = (currentUser == users.end()-1)?users.begin():currentUser+1;
        EV_DEBUG << "[ROUND_ROBIN] it's the turn of " << currentUser->getId() << endl;
    }
}


Frame* Antenna::vectorToFrame(std::vector<ResourceBlock> &v)
{
    Frame *f = new Frame();
    int c=0;
    for(auto it=v.begin(); it != v.end(); ++it)
    {
        f->setRBFrame(it - v.begin(), *it);
        c += (it->getRecipient() >= 0);
        //EV_DEBUG << "CONTO RB  "<< c << endl;
    }
    f->setAllocatedRBs(c);
    EV_DEBUG << "CONTO RB  "<< c << endl;

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
    if(NUM_USERS == 0) return;
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
            //packetsInformation[p->getId()].size      = packetSize;

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
                if(from->isAvailable()) {
                    currentUser->incrementNumberRBs();
                    from->allocResourceBlock(currentUserId, uCQI);
                }

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
            EV_DEBUG << "   FINAL INDEX:          " << (FRAME_SIZE) - (to - from) << endl;
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
    initRoundInformation();

    do
    {
        EV_DEBUG << "[CREATE_FRAME] Round Robin Starting Up..." <<endl;
        roundrobin();

        // Fill the frame with current user's queue and update currentRB index
        fillFrameWithCurrentUser(currentRB, vframe.end());
    } while(currentRB != vframe.end() && currentUser != lastUser );

    // 3) send the frame to all the users DURING NEXT TIMESLOT!
    this->frame = vectorToFrame(vframe);
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
        {
            EV_DEBUG << "[tptTest] Byte trasmitted from user"<< it->getId() <<": "<<it->getServedBytes()<< endl;
            if(it->getServedBytes()>0) // is this ok?????????? BOH
            {
                emit(it->throughput_s, it->getServedBytes());
                EV_DEBUG << "[tptTest] Emitted"<< endl;

            }
            emit(it->CQI_s, it->getCQI());
            emit(it->numberRBs_s, it->getNumberRBs());
            // emit(it->served_s, it->getNumServed());
        }
    }
    pendingPackets.clear(); // clear the pending packets data structure...
}


void Antenna::handlePacket(Packet *packet)
{
    EV_DEBUG << "[ANTENNA PKT] A new packet for user " << packet->getReceiverID() << endl;
    int userId = packet->getReceiverID();

    // this is a new packet! so we are going to keep its info somewhere!
    EV_DEBUG << "[UPLINK] Create a data structure for the new packet with ID " << packet->getId() << endl;
    Antenna::packet_info_t i;
    i.arrivalTime = simTime();
    i.served = false;
    i.recipient = userId;
    i.size = packet->getServiceDemand();

    EV_DEBUG << "[UPLINK] Inserting packet with ID " << packet->getId() << " in the packetsInformation hashmap" << endl;
    packetsInformation.insert(std::pair<long, Antenna::packet_info_t>(packet->getId(), i));

    EV_DEBUG << "[UPLINK] Inserting packet with ID " << packet->getId() << " in the Queue for the user " << userId << endl;
    EV_DEBUG << "[UPLINK] Getting the queue... " << endl;
    cQueue *q = users[userId].getQueue();
    EV_DEBUG << "[UPLINK] Queue: " << q << endl;

    EV_DEBUG << "[UPLINK] Inserting packet " << packet << endl;
    q->insert(packet);
    EV_DEBUG << "[UPLINK] INSERTED! "<< endl;
}


void Antenna::handleCQI(PacketCQI *notification)
{
    EV_DEBUG << " [ANTENA CQI] A new CQI Notification from user " << notification->getUserId() << endl;
    std::vector<UserInformation>::iterator u = users.begin() + notification->getUserId();
    u->setCQI(notification->getCQI());
    u->shouldBeServed();
    delete notification;
}


void Antenna::handleTimer(cMessage *msg)
{
    EV_DEBUG << " [ANTENA RR] Start propagation of the previous frame..." << endl;
    downlinkPropagation();

    EV_DEBUG << " [*** ANTENA RR ***] Create new frame..." << endl;
    createFrame();
    EV_DEBUG << " [*** ANTENA RR ***] DONE!" << endl;

    // Schedule next iteration
    simtime_t timeslot_dt = getParentModule()->par("timeslot");
    scheduleAt(simTime() + timeslot_dt, timer);
}

void Antenna::handleMessage(cMessage *msg)
{
    EV_DEBUG << "[ANTENNA] New message to be handled!" << endl;
    switch(msg->getKind())
    {
        case MSG_RR_TIMER:
            handleTimer(msg);
            break;

        case MSG_CQI:
            handleCQI(check_and_cast<PacketCQI*>(msg));
            break;

        case MSG_PKT:
            handlePacket(check_and_cast<Packet*>(msg));
            break;

        default:
            throw "This should never happen.";
    }
}


void Antenna::finish() {

    // Emit final signals
    for(auto it=users.begin(); it!=users.end(); ++it)
    {
        emit(it->served_s, it->getNumServed());
    }

    // Delete stuff...
    cancelAndDelete(timer);
    cancelAndDelete(frame);
}

Antenna::~Antenna()
{

}

