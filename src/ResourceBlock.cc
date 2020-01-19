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
    fragment_t fragment = {
        .id = p->getId(),
        .packetSize = p->getServiceDemand(),
        .fragmentSize = fragmentSize
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
