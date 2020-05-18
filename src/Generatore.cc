
#include "Generatore.h"

Define_Module(Generatore);

int Generatore::NEXT_USER_ID; // qua boh vedremo

void Generatore::initialize()
{
    userId = NEXT_USER_ID++;
    pt = new cMessage("timer");
    double spikeDisp = par("spike_displacement");

    scheduleAt(simTime() + (userId+1)*spikeDisp, pt);
}


void Generatore::generatePacket()
{

    EV_DEBUG << "[UPLINK] Create a new packet for user: " << userId << endl;
    Packet *packet = new Packet();

    EV_DEBUG << "[UPLINK] Adding some random service demand for the packet" << endl;
    packet->setServiceDemand(intuniform(MIN_SERVICE_DEMAND, MAX_SERVICE_DEMAND, RNG_SERVICE_DEMAND));
    packet->setKind(MSG_PKT);

    EV_DEBUG << "[UPLINK] Setting the recipient for the packet (" << userId <<")" << endl;
    packet->setReceiverID(userId);

    send(packet, "out");
}


void Generatore::handleMessage(cMessage *msg)
{
    simtime_t lambda = getParentModule()->par("lambda");
    generatePacket();
    scheduleAt(simTime() + exponential(lambda, RNG_INTERARRIVAL), msg);
}


void Generatore::finish()
{
 	cancelAndDelete(pt);
}
