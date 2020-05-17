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
    this->numberRBs = 0; // probabilmente da togliere
    this->numServed = 0; // probabilmente da togliere
}

long UserInformation::getNumServed() 
{
    return numServed;
}


int UserInformation::getNumberRBs()
{
    return numberRBs;
}

int UserInformation::getId()
{
    return id;
}

void UserInformation::setNumberRBs(int n)
{
    numberRBs = n;
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
    numberRBs = 0;
}

int UserInformation::CQIToBytes()
{
    int bytes[] = {3, 3, 6, 11, 15, 20, 25, 36, 39, 50, 63, 72, 80, 93, 93};
    return bytes[CQI-1];
}


void UserInformation::incrementNumberRBs() 
{
    numberRBs++;
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
    numServed++;
}


bool UserInformation::isServed()
{
    return served;
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

