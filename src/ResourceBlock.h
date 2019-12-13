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
        int sender;
        int recipient;
        std::vector<fragment_t> fragments;

    public:
        ResourceBlock();
        ResourceBlock(int sender, int recipient);
        ResourceBlock(const ResourceBlock &b);

        virtual void    setSender(int id);
        virtual void    setRecipient(int id);
        virtual int     getSender() const;
        virtual int     getRecipient() const;

        virtual void                      appendFragment(Packet* pkt, double fragmentSize);
        virtual int                       getNumFragments();
        virtual std::vector<fragment_t>   getFragments() const;
        virtual ResourceBlock::fragment_t getFragment(int i);
};


#endif /* RESOURCEBLOCK_H_ */
