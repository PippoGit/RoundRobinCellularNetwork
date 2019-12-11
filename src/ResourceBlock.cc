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

void ResourceBlock::appendPacket(Packet *p)
{
    contents.push_back(p);
}


Packet* ResourceBlock::getPacket(int i)
{
    if(contents.size() == 0) return nullptr;
    return contents[i];
}


ResourceBlock::ResourceBlock(const ResourceBlock &b)
{
    this->sender = b.getSender();
    this->recipient = b.getRecipient();
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


int ResourceBlock::getNumPackets()
{
    return contents.size();
}

std::vector<Packet*> ResourceBlock::getPackets()
{
    return contents;
}


/*
ResourceBlock::~ResourceBlock()
{
    //for(auto it:v)
    //{
        for(auto it:contents)
            delete it;
        //it.getPackets().clear();
    //}
}
*/

//double ResourceBlock::getRemainingPart(int id_receiver)
//{
//    if (id_receiver == this->receiverID)
//      return this->remainingPart;
//    else return 0;
//}

