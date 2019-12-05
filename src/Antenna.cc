#include "Antenna.h"

Define_Module(Antenna);

void Antenna::initialize()
{
    NUM_USERS = 10; // this->getParentModule()->par("numUsers");
    timer = new cMessage("timer");

    currentUser = 0;
    users.reserve(NUM_USERS);

    // Just fill the queues with random stuff....
    for(UserInformation u:users)
    {
        int i = uniform(0, 100);
        while(i++<100) {
            std::string name = "testPckt-" + i;
            cMessage *packet = new cMessage(name.c_str());
            cPar     *size   = new cPar("size");
            size->setIntValue(uniform(0, 100)); //bytes...
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


UserInformation* Antenna::roundrobin()
{
    return &users[(currentUser++ + 1)%NUM_USERS];
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


void Antenna::downlinkPropagation()
{
    double next_timeslot = simTime().dbl() + (double) this->par("timeslot");
    int currentRB = 0;
    std::vector<ResourceBlock> frame(25); // Frame *f = new Frame();

    // 1) Get updated CQIs
    updateCQIs();

    // 2) I guess i need to check all the queues?
    // THIS IS ACTUALLY PSEUDO-CODE... (TBD)
    do
    {
        int userId = currentUser;
        UserInformation *u = roundrobin();
        double remainingBytes = u->CQIToBytes();

        while(!u->getQueue()->isEmpty())
        {
            // Packet *p = check_and_cast<Packet*>(u->getQueue()->front());
            cMessage *p = check_and_cast<cMessage*>(u->getQueue()->front());
            EV << "I don't know what i'm doing..." << endl;
            EV << "   packet content: " << p->getName();

            if(currentRB + ceil(p->par("size")/u->CQIToBytes()) < FRAME_SIZE)
            {

                //NOT DONE YET:
                // 1) First check if there is enough space in the last occupied slot
                // 2) Then check if there is enough space in the frame
                // 3) Insert wherever there's enough space, and then update the currentRB index
                // 4) Update remaining bytes too


                // it fits
                EV << "it fits..." << endl;
                currentRB += ceil(p->par("size")/u->CQIToBytes());
                remainingBytes = currentRB - p->par("size")/u->CQIToBytes();
            }
        }

    } while(currentRB < FRAME_SIZE); // when currentRB is equal to 25 it means all frame is filled

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
