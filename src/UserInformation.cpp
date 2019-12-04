/*
 * UserInformation.cpp
 *
 *  Created on: 4 dic 2019
 *      Author: giado
 */

#include "UserInformation.h"

UserInformation::UserInformation()
{
    this->CQI = omnetpp::intuniform(rng, 1, 16);
    this->FIFOQueue = new cQueue();
}

void UserInformation::generateCQI()
{
    this->CQI = omnetpp::intuniform(rng, 1, 16); // tbd: per il momento facciamo uniform
}

UserInformation::~UserInformation()
{
    // idk if this is ok (i kinda forgot most of the shit about c++)
    delete this->FIFOQueue;
}

