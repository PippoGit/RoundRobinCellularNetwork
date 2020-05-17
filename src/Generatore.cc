
#include "Generatore.h"

Define_Module(Generatore);

int Generatore::NEXT_USER_ID; // qua boh vedremo

void Generatore::initialize()
{
    // TODO - Generated method body
    outMSG_s = gate("outMSG");
    userId = NEXT_USER_ID++;
    pt = new PacketTimer();
    pt->setKind(MSG_PKT_TIMER);
    pt->setUserId(userId);
    generatePacket();
}

void Generatore::generatePacket()
{
    int userId = pt->getUserId()();
    EV_DEBUG << "[UPLINK] Create a new packet for user: " << userId << endl;
    Packet *packet = new Packet();

    if (packet != nullptr)
        EV_DEBUG << "the packet is generated:" << packet << endl;

    EV_DEBUG << "[UPLINK] Adding some random service demand for the packet" << endl;
    packet->setServiceDemand(intuniform(MIN_SERVICE_DEMAND, MAX_SERVICE_DEMAND, RNG_SERVICE_DEMAND));
    packet->setKind(MSG_PKT);
    EV_DEBUG << "[UPLINK] Setting the recipient for the packet (" << userId <<")" << endl;
    packet->setReceiverID(userId);
    send(packet,outMSG_s);
    double timeslot = getParentModule()->par("timeslot");
    scheduleAt(simTime() + timeslot,pt);

}
void Generatore::handleMessage(cMessage *msg)
{
    generatePacket();

}
