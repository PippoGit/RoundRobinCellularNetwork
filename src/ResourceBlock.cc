/*
 * ResourceBlock.cc
 *
 *  Created on: 5 dic 2019
 *      Author: giada
 */

#include "ResourceBlock.h"


ResourceBlock::ResourceBlock()
{
    this->sender    = -1;
    this->recipient = -1;
}

ResourceBlock::ResourceBlock(int sender, int recipient)
{
    this->sender = sender;
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
}


ResourceBlock::fragment_t ResourceBlock::getFragment(int i)
{
    return fragments[i];
}


ResourceBlock::ResourceBlock(const ResourceBlock &b)
{
    this->sender    = b.getSender();
    this->recipient = b.getRecipient();
    this->fragments = b.getFragments();
}


void ResourceBlock::setRecipient(int id)
{
    recipient = id;
}


void ResourceBlock::setSender(int id)
{
    sender = id;
}


int ResourceBlock::getSender() const
{
    return sender;
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
