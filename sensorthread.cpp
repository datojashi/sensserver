#include "sensorthread.h"


SensorThread::SensorThread()
{
    for(int i=0; i<MAX_AUDIO_CHANNELS; i++)
    {
        audiochannels[i] = new AudioChannel(i,"/home/dato/sensserver_test");
    }
}

SensorThread::~SensorThread()
{
    for(int i=0; i<MAX_AUDIO_CHANNELS; i++)
    {
        delete audiochannels[i];
    }
}

void SensorThread::onstart()
{
    ct=0;
    ct2=0;
    ct3=0;
    chunk.clear();
    messages.clear();
}



void SensorThread::processAudioData()
{
    awl::ByteArray adata=awl::Core::byteArrayRight(message,sizeof (MESSAGE));
    for(unsigned int i=0; i<adata.size(); i++)
    {

    }
}

void SensorThread::onwork()
{
    //std::cout << "onwork "  << msg_ct << std::endl;
    if(msg_ct > 100)
    {
        char resp[256];
        memset(resp,0,256);
        resp[1]=cmd_ping_request;
        resp[2]=send_ct++;
        awl::ByteArray ba;
        awl::Core::initba(ba,resp,256);
        socket->send(ba);
        msg_ct=0;
    }
}

void SensorThread::onmessage()
{
    MESSAGE* msg = (MESSAGE*)message.data();

    std::cout << "=== ";
    awl::Core::printhex(message);


    //*
    switch(msg->cmd)
    {
    case cmd_ping_request:
    {
        char resp[256];
        memset(resp,0,256);
        int n = std::sprintf(resp,"Response on Ping: %d", msg->nmb);
        //if(n < 256)
        {
            std::cout << "PONG   " << msg->nmb << '\t' << messages.size() << '\t' << std::string(resp) << std::endl;
            msg_ct=0;
            usleep(10000);
            //socket->send(resp,256);
        }
        break;
    }
    case cmd_AudioData_request:
    {
        msg_ct++;
        std::cout << " --- " <<   (unsigned int)msg->cmd << "\t" << (unsigned int)msg->nmb << "\t" << msg->sz << std::endl;
        uint nmb=(unsigned int)msg->nmb;
        std::cout << "===" << nmb << std::endl;

        if(nmb > 0 && nmb <= MAX_AUDIO_CHANNELS)
        {
            message=awl::Core::byteArrayRight(message,sizeof(MESSAGE));
            int cind=0;

            for(unsigned int i=0; i<message.size(); i++)
            {
                char d=message.at(i);//^cypher[cind];
                if(++cind==16) cind=0;
                audiochannels[i%nmb]->data.push_back(d);
            }

            for(uint i=0; i<nmb;i++)
            {
                audiochannels[i]->saveData();
            }
        }
        break;
    }
    default:
    {
        std::cout << "Unknown message from sensor: " << msg->cmd << " --- ";
        awl::Core::printhex(message);
    }
    }
    //*/
}

/*
void SensorThread::getmessage()
{
    if(tba.size() > 0)
    {
        messages.clear();
        unsigned int  i=0;
        if(chunk.size() > 0)
        {
            awl::Core::prependByteArray(tba,chunk);
            chunk.clear();
        }
        awl::ByteArray msg;


        while( (i+524) <= tba.size())
        {
            if((unsigned char)tba.at(i)==0xaa && (unsigned char)tba.at(i+1)==0x55 && (unsigned char)tba.at(i+2)==0xaa && (unsigned char)tba.at(i+3)==0x55)
            {
                msg=awl::Core::byteArrayMid(tba,i,524);
                messages.push_back(msg);
                //std::cout << "\t===== " << i << "\t==== " << msg.size() << "\t==== ";
                //awl::Core::printhex(msg);
            }
            else
            {
                i++;
                continue;
            }

            i=i+524;
        }
        if(i < tba.size())
        {
            chunk=awl::Core::byteArrayRight(tba,i);
        }


        ct=ct+messages.size();
        ct2=ct2+messages.size();


    }
}
*/

void SensorThread::getmessage()
{
    //*
    if(chunk.size() > 0)
    {
        awl::Core::prependByteArray(tba,chunk);
        chunk.clear();
    }
    unsigned int  i=0;
    MESSAGE* msgp;
    awl::ByteArray msg;

    //std::cout << "====== 2 ====== " << tba.size() << std::endl;
    messages.clear();
    size=0;
    while( i+sizeof (MESSAGE) < tba.size() )
    {
        msgp=(MESSAGE*)(tba.data()+i);
        if(msgp->tag==0x55aa)
        {
            //std::cout << "====== 3 ====== " << msgp->cmd << "\t" << msgp->nmb << "\t" << msgp->sz << std::endl;
            if( (i+msgp->sz+sizeof (MESSAGE)) < tba.size() )
            {
                msg=awl::Core::byteArrayMid(tba,i,msgp->sz);
                //awl::Core::printhex(msg);
                messages.push_back(msg);
                i=i+msgp->sz+sizeof (MESSAGE);
                continue;
            }
            else
            {
                chunk=awl::Core::byteArrayRight(tba,i);
                break;
            }
        }
        else
        {
            i++;
            continue;
        }
    }
    //*/


}
