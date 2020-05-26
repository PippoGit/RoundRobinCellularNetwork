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
omnetpp::simsignal_t UserInformation::getNqSignal()
{
    return nq_s;
}
int UserInformation::getId()
{
    return id;
}

void UserInformation::setCQI(int cqi)
{
    CQI = cqi;
}
int UserInformation::getCQI()
{
    return CQI;
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


omnetpp::cQueue* UserInformation::getQueue()
{
    return &FIFOQueue;
}


void UserInformation::setNqSignal(omnetpp::simsignal_t nq_s)
{
    this->nq_s = nq_s;
}

UserInformation::~UserInformation()
{
    // idk if this is ok (i kinda forgot most of the shit about c++)
    //delete this->FIFOQueue;
}
