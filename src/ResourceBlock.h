/*
 * ResourceBlock.h
 *
 *  Created on: 5 dic 2019
 *      Author: giada
 */

#ifndef RESOURCEBLOCK_H_
#define RESOURCEBLOCK_H_

#include <omnetpp.h>
#include "Packet_m.h"

class ResourceBlock {
    public:
        struct fragment_t {
            long   id;
            int    packetSize;
            double fragmentSize;
        };

    private:
        int recipient;
        std::vector<fragment_t> fragments;
        double remainingBytes;

    public:
        ResourceBlock();
        ResourceBlock(int recipient);
        ResourceBlock(const ResourceBlock &b);

        virtual void    setRecipient(int id);
        virtual int     getRecipient() const;

        virtual void                      appendFragment(Packet* pkt, double fragmentSize);
        virtual int                       getNumFragments();
        virtual std::vector<fragment_t>   getFragments() const;
        virtual ResourceBlock::fragment_t getFragment(int i);

        virtual void                      allocResourceBlock(int user, int cqiBytes);

        bool  isAvailable()        { return (recipient == -1);   };
        bool  isFull()             { return (remainingBytes==0); };
        double getRemainingBytes() { return remainingBytes;      };
};


#endif /* RESOURCEBLOCK_H_ */
