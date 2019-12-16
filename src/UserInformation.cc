/*
 * UserInformation.cpp
 *
 *  Created on: 4 dic 2019
 *      Author: giado
 */

#include "UserInformation.h"
// int UserInformation::NEXT_USER_ID;

UserInformation::UserInformation()
{
    // id = NEXT_USER_ID++; // this thing should work because stuff don't change over time... (iguess)
    CQI = 0; //before FIRST utilization the CQI should be generated
    numPendingPackets = 0;
}

/*
int UserInformation::getUserId()
{
    return id;
}
*/


int UserInformation::getNumPendingPackets()
{
    return numPendingPackets;
}

void UserInformation::incrementNumPendingPackets()
{
    numPendingPackets++;
}


void UserInformation::setNumPendingPackets(int val)
{
    numPendingPackets = val;
}

void UserInformation::initNumPendingPackets()
{
    setNumPendingPackets(0);
}

int UserInformation::CQIToBytes()
{
    int bytes[] = {3, 3, 6, 11, 15, 20, 25, 36, 39, 50, 63, 72, 80, 93, 93};
    return bytes[CQI-1];
}


void UserInformation::generateCQI(omnetpp::cRNG*seedCQI)
{
    CQI = omnetpp::intuniform(seedCQI, MIN_CQI, MAX_CQI);
    remainingBytes = CQIToBytes();
}


omnetpp::cQueue* UserInformation::getQueue()
{
    return &FIFOQueue;
}


UserInformation::~UserInformation()
{
    // idk if this is ok (i kinda forgot most of the shit about c++)
    //delete this->FIFOQueue;
}

