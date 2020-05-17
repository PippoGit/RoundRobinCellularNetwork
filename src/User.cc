#include "User.h"

Define_Module(User);

int User::NEXT_USER_ID;

void User::initialize()
{
    userID = NEXT_USER_ID++;

    pt = new PacketTimer();
    pt->setKind(MSG_PKT_TIMER);
    pt->setUserId(userID);

    sendCQI();
}

void User::sendCQI(){
    simtime_t lambda = getParentModule()->par("lambda");
    bool isBinomial = getParentModule()->par("isBinomial");
    double successProbGroup1 = getParentModule()->par("successProbGroup1");
    double successProbGroup2 = getParentModule()->par("successProbGroup2");
    double successProbGroup3 = getParentModule()->par("successProbGroup3");
    double timeslot = getParentModule()->par("timeslot");
    double p =  (userID % 2 == 0)? successProbGroup3: successProbGroup1;
    int cqi = (isBinomial)?binomial(BINOMIAL_N, p,RNG_CQI_BIN)+1:intuniform(MIN_CQI, MAX_CQI, RNG_CQI_UNI);

    PacketCQI *newCQI = new PacketCQI();
    newCQI->setUserID(userID);
    newCQI->setCqi(cqi);
    newCQI->setKind(MSG_CQI);

    send(newCqi, "out");

    scheduleAt(simTime() + timeslot,pt);



}

void User::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()){
        sendCQI();
    }
    else{
        Frame *f = check_and_cast<Frame*>(msg);
        handleFrame(f);
    }
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
                EV << "   ID PKT:   " << f->getRBFrame(i).getFragment(j).id << endl;
                EV << "   PKT SIZE: " <<  f->getRBFrame(i).getFragment(j).packetSize << endl;
                EV << "   FRG SIZE: " <<  f->getRBFrame(i).getFragment(j).fragmentSize << endl;
            }
        }
    }
    delete(f);
}


