#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    NUM_USERS = 10; // this->getParentModule()->par("numUsers");
    users = new UserInformation[NUM_USERS];
    timer = new cMessage("timer");
    currentUser = 0;
}

void Antenna::updateCQIs()
{
    for(UserInformation *u : users)
        u->generateCQI();
}


UserInformation* Antenna::roundrobin()
{
    return users[(currentUser++ + 1)%NUM_USERS];
}


bool Antenna::frameFull()
{
    return false;
}


void Antenna::broadcastFrame(Frame *f)
{
    // for simplicity just send it to every users and then the user will check
    // if there is something for them.
    for(int i=0; i<NUM_USERS; i++)
    {
        Frame *copy = f->dup();
        send(copy, "out", i);
    }
    delete f;
}


void Antenna::downlinkPropagation()
{
    double next_timeslot = simTime().dbl() + this->par("timeslot");
    int currentRB = 0;
    Frame *f = new Frame();

    // 1) Get updated CQIs
    updateCQIs();

    // 2) I guess i need to check all the queue?
    // THIS IS ACTUALLY PSEUDO-CODE... (TBD)
    do
    {
        int userId = currentUser;
        UserInformation *u = roundrobin();
        double remainingBytes = u->CQIToBytes();

        while(!u->getQueue()->empty())
        {
            Packet *p = u->getQueue()->head();

            if(currentRB + ceil(p->getSize()/u->CQIToBytes()) < FRAME_SIZE)
            {
                // it fits

            }
        }

    } while(!frameFull());

    // 3) send the frame to all the users
    broadcastFrame(f);

    // Schedule next iteration
    scheduleAt(next_timeslot, timer);
}


void Antenna::handlePacket(Packet *p)
{

}


void Antenna::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) // TEMPORARY ASSUMPTION: The frame is READY before the next
        downlinkPropagation();
    else
        handlePacket(check_and_cast<Frame>(msg));
}
