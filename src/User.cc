#include "User.h"

Define_Module(User);

int User::NEXT_USER_ID;


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

    // Init stats
    numberRBs   = 0;
    numServed   = 0;
    servedBytes = 0;

    // Init signals
    throughput_s   = createDynamicSignal("TESTtptUser", "tptUserTemplate");
    responseTime_s = createDynamicSignal("TESTrspTimeUser", "responseTimeUserTemplate");
    CQI_s          = createDynamicSignal("TESTCQIUser", "CQIUserTemplate");
    numberRBs_s    = createDynamicSignal("TESTNumRBUser", "numberRBsUserTemplate");
    served_s       = createDynamicSignal("TESTservedUser", "servedUserTemplate");

    scheduleAt(simTime(), pt);
}

void User::sendCQI(){
    simtime_t lambda = getParentModule()->par("lambda");
    bool isBinomial = getParentModule()->par("isBinomial");
    double successProbGroup1 = getParentModule()->par("successProbGroup1");
    double successProbGroup2 = getParentModule()->par("successProbGroup2");
    double successProbGroup3 = getParentModule()->par("successProbGroup3");
    double timeslot = getParentModule()->par("timeslot");

    double p =  (userID % 2 == 0) ? successProbGroup3: successProbGroup1;
    int cqi  = (isBinomial) ? binomial(BINOMIAL_N, p,RNG_CQI_BIN)+1 : intuniform(MIN_CQI, MAX_CQI, RNG_CQI_UNI);

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
        simtime_t timeslot = getParentModule()->par("timeslot");
        sendCQI();
        scheduleAt(simTime() + timeslot, msg);
    }
    else
    {
        Frame *f = check_and_cast<Frame*>(msg);
        handleFrame(f);
    }
}



void User::handleFrame(Frame* f)
{
    EV_DEBUG << "[USER] I have received a frame... Here is the content:" << endl;
    long servedBytesRound = 0;
    long numberRBsRound   = 0;
    int lastSeen = -1;

    for(int i =0; i<FRAME_SIZE; i++)
    {
        if(f->getRBFrame(i).getRecipient()==userID)
        {
            EV_DEBUG << "[USER] The frame is for me! yay " << "Num Time Served: " << numServed << endl;
            if (simTime() > getSimulation()->getWarmupPeriod()) {

                // per ogni frammento, se è di un pacchetto nuovo emitto le info, altirmenti scorri
                for(auto frag:f->getRBFrame(i).getFragments()) {
                    EV_DEBUG << "[USER] Last seen frag: " << lastSeen << endl;

                    if (lastSeen != frag.id) {
                        EV_DEBUG << "[USER] Emitting info about packet with id " << frag.id << endl;

                        lastSeen = frag.id;
                        // Global Stats
                        numServed++;
                        numberRBs++;
                        servedBytes += frag.packetSize;

                        // Round Stats
                        servedBytesRound += frag.packetSize; // this is set to zero at every round
                        numberRBsRound++;
                        emit(responseTime_s, simTime() - frag.arrivalTime);
                    }
                }
            }
        }
    }

    // Emitto statistiche per questo round
    emit(throughput_s, servedBytesRound);
    emit(numberRBs_s, numberRBsRound);

    delete(f);
}


void User::finish()
{
    // Emit globali
    emit(served_s, numServed);

    cancelAndDelete(pt);
}

