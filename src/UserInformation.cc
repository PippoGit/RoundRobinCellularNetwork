/*
 * UserInformation.cpp
 *
 *  Created on: 4 dic 2019
 *      Author: giado
 */

#include "UserInformation.h"

UserInformation::UserInformation()
{
    FIFOQueue = new omnetpp::cQueue();
    remainingBytes = CQIToBytes();
    CQI = 0; //before FIRST utilization the CQI should be generated
}

int UserInformation::CQIToBytes()
{
    int bytes[] = {3, 3, 6, 11, 15, 20, 25, 36, 39, 50, 63, 72, 80, 93, 93};
    return bytes[CQI-1];
}

void UserInformation::generateCQI()
{
    omnetpp::cRNG* seedCQI = omnetpp::cComponent::getRNG(SEED_CQI);
    CQI = omnetpp::intuniform(seedCQI, MIN_CQI, MAX_CQI);
}


omnetpp::cQueue* UserInformation::getQueue()
{
    return FIFOQueue;
}


std::vector<Packet*> UserInformation::getPackets(int available)
{
//    // IDEA: return as much packets as i can fit
      std::vector<Packet*> pkts;
//    while(!FIFOQueue->empty())
//    {
//        Packet *p = FIFOQueue->head();
//        if(available < ceil(p->getSize()/CQIToBytes(CQI))
//        {
//            pkts.push_back(FIFOQueue->pop());
//            available -= 1;
//        }
//    }
//    return pkts;
    return pkts;
}

UserInformation::~UserInformation()
{
    // idk if this is ok (i kinda forgot most of the shit about c++)
    delete this->FIFOQueue;
}

