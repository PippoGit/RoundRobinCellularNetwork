/*
 * ResourceBlock.cc
 *
 *  Created on: 5 dic 2019
 *      Author: giada
 */

#include "ResourceBlock.h"


ResourceBlock::ResourceBlock(){}

double ResourceBlock::getRemainingPart(int id_receiver)
{
    if (id_receiver == this->receiverID)
      return this->remainingPart;
    else return 0;
}

