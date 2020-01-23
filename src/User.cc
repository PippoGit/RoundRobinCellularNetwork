#include "User.h"

Define_Module(User);

int User::NEXT_USER_ID;

void User::initialize()
{
    userID = NEXT_USER_ID++;

}

void User::handleMessage(cMessage *msg)
{
        Frame *f = check_and_cast<Frame*>(msg);
        handleFrame(f);
}



void User::handleFrame(Frame* f)
{
    EV << "[USER] I have received a frame... Here is the content:" << endl;
    for(int i =0; i<FRAME_SIZE; i++)
    {
        if(f->getRBFrame(i).getRecipient()==userID)
        {
            int numFragments = f->getRBFrame(i).getNumFragments();
            EV << "[USER] There are " << numFragments << " fragments" << endl;
            for(int j = 0; j < numFragments; j++)
            {
                EV << "   ID PKT:   " << f->getRBFrame(i).getFragment(j).id << endl;
                EV << "   PKT SIZE: " <<  f->getRBFrame(i).getFragment(j).packetSize << endl;
                EV << "   FRG SIZE: " <<  f->getRBFrame(i).getFragment(j).fragmentSize << endl;
            }
        }
    }
    delete(f);
}


