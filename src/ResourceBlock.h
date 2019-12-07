/*
 * ResourceBlock.h
 *
 *  Created on: 5 dic 2019
 *      Author: giada
 */

#ifndef RESOURCEBLOCK_H_
#define RESOURCEBLOCK_H_

#include <omnetpp.h>

class ResourceBlock {
    private:
          int sender; // this is probably useless...
          int recipient;

          // bool lastForUser;
          // double remainingPart;

    public:
        ResourceBlock();
        ResourceBlock(int sender, int recipient);
        ResourceBlock(const ResourceBlock &b);

        virtual void setSender(int id);
        virtual void setRecipient(int id);
        virtual int  getSender() const;
        virtual int  getRecipient() const;

        // virtual double getRemainingPart(int id_receiver);
};


#endif /* RESOURCEBLOCK_H_ */
