/*
 * UserInformation.cpp
 *
 *  Created on: 4 dic 2019
 *      Author: giado
 */

#include "UserInformation.h"

UserInformation::UserInformation()
{
    generateCQI();
}

void UserInformation::generateCQI()
{
    this->CQI = omnetpp::intuniform(rng, 1, 16); // tbd: per il momento facciamo uniform
}

UserInformation::~UserInformation()
{
    // TODO Auto-generated destructor stub
}

