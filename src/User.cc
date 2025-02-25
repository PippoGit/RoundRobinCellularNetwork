#include "User.h"

Define_Module(User);

int User::NEXT_USER_ID;

// i know that this function is dumb, but it's the best way to avoid modifying the python script (mybad)
simsignal_t User::createDynamicSignal(std::string prefix, std::string templateName)
{
    simsignal_t signal;
    std::string signal_name;
    std::stringstream sstream;

    sstream << prefix << "-" << userID;
    signal_name = sstream.str();

    signal = registerSignal(signal_name.c_str());

    cProperty *statisticTemplate = getProperties()->get("statisticTemplate", templateName.c_str());
    getEnvir()->addResultRecorders(this, signal, signal_name.c_str(), statisticTemplate);
    return signal;
}


void User::initialize()
{
    userID = NEXT_USER_ID++;
    pt = new cMessage("timer");
    timeslot = getParentModule()->par("timeslot");

    // Init signals
    throughput_s      = createDynamicSignal("tptUser", "tptUserTemplate");
    responseTime_s    = createDynamicSignal("rspTimeUser", "responseTimeUserTemplate");
    waitingTime_s     = createDynamicSignal("waitingTimeUser", "waitingTimeUserTemplate");
    serviceTime_s     = createDynamicSignal("serviceTimeUser", "serviceTimeUserTemplate");
    turnWaitingTime_s = createDynamicSignal("turnWaitingTimeUser", "turnWaitingTimeUserTemplate");
    CQI_s             = createDynamicSignal("CQIUser", "CQIUserTemplate");
    numberRBs_s       = createDynamicSignal("numRBsUser", "numberRBsUserTemplate");
    numberPkts_s      = createDynamicSignal("numPktsUser", "numberPktsUserTemplate");
    served_s          = createDynamicSignal("servedUser", "servedUserTemplate");
    scheduleAt(simTime(), pt);
}

void User::sendCQI(){
    bool isBinomial = getParentModule()->par("isBinomial");
    double successProbGroup1 = getParentModule()->par("successProbGroup1");
    //double successProbGroup2 = getParentModule()->par("successProbGroup2");
    double successProbGroup3 = getParentModule()->par("successProbGroup3");

    //double p = (userID==2||userID==5||userID==8)? successProbGroup1: (userID==1||userID==4||userID==7||userID==9)? successProbGroup2: successProbGroup3; //caso 3 prob
    //double p =   (userID==2)?successProbGroup3:successProbGroup1;
    double p =  (userID % 2 == 0) ? successProbGroup3: successProbGroup1;

    int cqi  = (isBinomial) ? binomial(BINOMIAL_N, p,RNG_CQI_BIN)+1 : intuniform(MIN_CQI, MAX_CQI, RNG_CQI_UNI);
    // TEST PER MULTI CQI int cqi  = userID+1; //(isBinomial) ? binomial(BINOMIAL_N, p,RNG_CQI_BIN)+1 : intuniform(MIN_CQI, MAX_CQI, RNG_CQI_UNI);

    PacketCQI *newCQI = new PacketCQI();
    newCQI->setUserId(userID);
    newCQI->setCQI(cqi);
    newCQI->setKind(MSG_CQI);

    // Emit del CQI per le statistiche
    emit(CQI_s, cqi);

    send(newCQI, "out");
}


void User::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()){
        sendCQI();
        scheduleAt(simTime() + timeslot, msg);
    }
    else
    {
        Frame *f = check_and_cast<Frame*>(msg);
        handleFrame(f);
    }
}

void User::inspectResourceBlock(const ResourceBlock &rb, rb_inspection_result_t &res)
{
    if(rb.getRecipient() != userID) return; // not my cup of tea

    EV_DEBUG << "[USER] There is a RB for me. Last seen pkt: " << res.last_seen << endl;
    res.number_rbs++;

    for(auto frag : rb.getFragments())
    {
        if (res.last_seen != frag.id) 
        {
            EV_DEBUG << "[USER] Recording info about packet with id " << frag.id << endl;

            // Update Stats
            res.last_seen = frag.id;
            res.served_bytes += frag.packetSize;
            res.number_pkts++;

            emit(responseTime_s, simTime() - frag.arrivalTime);
            emit(waitingTime_s, frag.servedTime - frag.arrivalTime);    // period between arrival time and the moment when the packet is considered for the first time
            emit(turnWaitingTime_s, frag.frameTime - frag.servedTime);  // period between when it was considered for the first time and when it was put in a frame
            emit(serviceTime_s, simTime() - frag.frameTime);            // period between when it was put in the frame and when it was propagated (should be always 1ms)
        }
    }
}

void User::handleFrame(Frame* f)
{
    EV_DEBUG << "[USER] I have received a frame" << endl;
    rb_inspection_result_t res;

    if(simTime() > getSimulation()->getWarmupPeriod())
    {
        for(int i=0; i<FRAME_SIZE; i++)
        {
            ResourceBlock rb = f->getRBFrame(i);
            inspectResourceBlock(rb, res);
        }

        // Emitto statistiche per questo round
        emit(served_s, (int) !(res.last_seen < 0));
        emit(throughput_s, (double) res.served_bytes/timeslot);
        emit(numberRBs_s, res.number_rbs);
        emit(numberPkts_s, res.number_pkts);

    }

    // delete del frame ora che ï¿½ stato consumato
    delete(f);
}


void User::finish()
{
    cancelAndDelete(pt);
}

