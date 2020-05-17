/*
 * ResourceBlock.cc
 *
 *  Created on: 5 dic 2019
 *      Author: giada
 */

#include "ResourceBlock.h"


ResourceBlock::ResourceBlock()
{
    this->recipient = -1;
}

ResourceBlock::ResourceBlock(int recipient)
{
    this->recipient = recipient;
}

void ResourceBlock::appendFragment(Packet *p, double fragmentSize)
{
    // I know, this is not very efficient, but it works. So it's fine.
    // For semplicity everytime we insert a new fragment we take all the info
    // that we are going to need at the user and we copy them into the fragment
    // so that when we receive stuff we can eval everything with no problem (i hope)
    fragment_t fragment = {
        .id = p->getId(),
        .packetSize = p->getServiceDemand(),
        .fragmentSize = fragmentSize,

        // to evaluate stuff at the user
        .arrivalTime = p->getArrivalTime(),     // inserted into the queue
        .servedTime  = p->getServedTime(),      // removed from the queue
        .frameTime   = p->getFrameTime()       // inserted into the frame
    };

    fragments.push_back(fragment);
    this->remainingBytes -= fragmentSize;
}


ResourceBlock::fragment_t ResourceBlock::getFragment(int i)
{
    return fragments[i];
}


ResourceBlock::ResourceBlock(const ResourceBlock &b)
{
    this->recipient = b.getRecipient();
    this->fragments = b.getFragments();
}


void ResourceBlock::setRecipient(int id)
{
    recipient = id;
}


void ResourceBlock::allocResourceBlock(int user, int cqiBytes)
{
    recipient = user;
    remainingBytes = cqiBytes;
}


int ResourceBlock::getRecipient() const
{
    return recipient;
}


int ResourceBlock::getNumFragments()
{
    return fragments.size();
}

std::vector<ResourceBlock::fragment_t> ResourceBlock::getFragments() const
{
    return fragments;
}
