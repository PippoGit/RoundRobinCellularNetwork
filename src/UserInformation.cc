/*
 * UserInformation.cpp
 *
 *  Created on: 4 dic 2019
 *      Author: giado
 */

#include "UserInformation.h"

UserInformation::UserInformation(int id)
{
    this->id = id;
    this->CQI = 0;
    this->numPendingPackets = 0;
}

void UserInformation::setTimer(PacketTimer* t)
{
    timer = t;
}


int UserInformation::getId()
{
    return id;
}

void UserInformation::setCQI(int cqi)
{
    CQI = cqi;
}

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

void UserInformation::shouldBeServed() 
{
    initNumPendingPackets();
    served = false;
    servedBytes = 0;
}

int UserInformation::CQIToBytes()
{
    int bytes[] = {3, 3, 6, 11, 15, 20, 25, 36, 39, 50, 63, 72, 80, 93, 93};
    return bytes[CQI-1];
}


void UserInformation::incrementServedBytes(int bytes)
{
    servedBytes += bytes;
}


int UserInformation::getServedBytes()
{
    return servedBytes;
}


void UserInformation::serveUser()
{
    served = true;
}


bool UserInformation::isServed()
{
    return served;
}


void UserInformation::generateCQI(omnetpp::cRNG*RNG, bool isBinomial)
{
    if(isBinomial)
    {
        CQI = omnetpp::binomial(RNG, MIN_CQI, MAX_CQI);
    }
    else
    {
        CQI = omnetpp::intuniform(RNG, MIN_CQI, MAX_CQI);
    }
    // remainingBytes = CQIToBytes();
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

