/*
 * ResourceBlock.h
 *
 *  Created on: 5 dic 2019
 *      Author: giada
 */

#ifndef RESOURCEBLOCK_H_
#define RESOURCEBLOCK_H_

#include <omnetpp.h>
#include <cqueue.h>
#include <crandom.h>

class ResourceBlock {
    private:
          int receiverID;
          bool lastForUser;
          double remainingPart;

    public:
        ResourceBlock();
        virtual double getRemainingPart(int id_receiver);






};


#endif /* RESOURCEBLOCK_H_ */
