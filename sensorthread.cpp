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


bool SensorThread::addCommand(COMMAND cmd, bool mode)
{
    mutex.lock();
    if(mode)
    {
        commands.push(cmd);
        std::cout << "Add cmd " << cmd.cmd << std::endl;
    }
    else if(commands.empty())
    {
        commands.push(cmd);
        std::cout << "Add cmd PING " << cmd.cmd << std::endl;
    }


    mutex.unlock();
    return true;
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

void SensorThread::processCommand()
{
    COMMAND cmd;
    cmd.cmd=cmd_None;

    if(mutex.try_lock())
    {
        if(!commands.empty())
        {
            cmd=commands.front();
            commands.pop();
        }
        mutex.unlock();
    }
    if(cmd.cmd!=cmd_None)
    {
        sendMsg(cmd);
        msgState.store(ms_sent);
        //std::cout << "msgState=" << msgState.load() << std::endl;
    }
}

time_t SensorThread::setCurrentTime()
{
    time_t t=time(0);
    struct tm *dt=localtime(&t);
    reqdata[3]=dt->tm_sec;
    reqdata[4]=dt->tm_min;
    reqdata[5]=dt->tm_hour;
    reqdata[6]=dt->tm_mday;
    reqdata[7]=dt->tm_mon+1;
    reqdata[8]=dt->tm_year-100;
    //9,10,11 - reserve
    return t;

}

uint32_t SensorThread::getSectorByTime(time_t t)
{
    std::cout << "&&&&&&&&&" << t << '\t' << sensTime << std::endl;
    uint32_t result;
    uint32_t secs = t - sensTime;
    result=DATA_SECTOR+43*secs;
    return result;
}

void SensorThread::sendMsg(COMMAND cmd)
{
    memset(reqdata,0,64);
    reqdata[1]=cmd.cmd;
    reqdata[2]=send_ct++;

    switch (cmd.cmd)
    {
    case cmd_ping_request:
    {
        std::cout << "Ping sent" << std::endl;
        setCurrentTime();
        waitmsg=cmd_ping_response;
        break;
    }
    case cmd_AudioData_response:
    {
        setCurrentTime();
        break;
    }
    case cmd_startAudio_request:
    {
        if(cmd.t0==0 || cmd.t0<=sensTime)
        {
            cmd.t0=sensTime;
        }
        time_t t=time(0);
        if(cmd.t1==0 || cmd.t1 > t )
        {
            cmd.t1=t;
        }
        if(cmd.t0 >= cmd.t1)
        {
            cmd.t0=sensTime;
            cmd.t1=t;
        }
        *((uint32_t*)(reqdata+4))=getSectorByTime(cmd.t0);
        *((uint32_t*)(reqdata+8))=getSectorByTime(cmd.t1);
        std::cout << " ===== Start adio request sent" << std::endl;
        waitmsg=cmd_startAudio_response;
        break;
    }
    case cmd_stopAudio_request:
    {
        std::cout << " ===== Stop adio request sent" << std::endl;
        waitmsg=cmd_stopAudio_response;
        break;
    }
    case cmd_startLive_request:
    {
        waitmsg=cmd_startLive_response;
        break;
    }
    case cmd_stopLive_request:
    {
        waitmsg=cmd_stopLive_response;
        break;
    }
    case cmd_setConfig_request:
    {
        sensTime=setCurrentTime();
        *((uint32_t*)(reqdata+12))=SETTINGS_SECTOR;
        *((uint32_t*)(reqdata+16))=CLOCK_SECTOR;
        *((uint32_t*)(reqdata+20))=DATA_SECTOR;
        *((uint32_t*)(reqdata+24))=BAUD_RATE;
        std::cout << "Set RTC request sent   " << msgState.load() << std::endl;
        waitmsg=cmd_setConfig_response;
        break;
    }
    default:
        break;
    }

    awl::ByteArray ba;
    awl::Core::initba(ba,reqdata,64);
    //awl::Core::printhex(ba);
    socket->send(ba);
}

void SensorThread::onwork()
{
    if(msgState.load()==ms_none)
    {
        processCommand();
    }
}

void SensorThread::onmessage()
{
    MESSAGE* msg = (MESSAGE*)message.data();

    //std::cout << "=== ";
    //awl::Core::printhex(message);


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
    case cmd_ping_response:
    {
        if(msgState.load()==ms_sent && waitmsg==cmd_ping_response)
        {
            msgState.store(ms_responded);
            //std::cout << "Message ping_response received" << std::endl;
        }
        break;
    }
    case cmd_setConfig_response:
    {
        if(msgState.load()==ms_sent && waitmsg==cmd_setConfig_response)
        {
            msgState.store(ms_responded);
            std::cout << "Message setRTC_response received" << std::endl;
        }
        else
        {
            std::cout << "Message setRTC_response not excepted" << std::endl;

        }

        break;
    }
    case cmd_startAudio_response:
    {
        msgState.store(ms_responded);
        std::cout << "Message cmd_startAudio_response received" << std::endl;
        break;
    }
    case cmd_stopAudio_response:
    {
        msgState.store(ms_responded);
        std::cout << "Message cmd_stopAudio_response received" << std::endl;
        break;
    }
    case cmd_startLive_request:
    {
        break;
    }
    case cmd_stopLive_response:
    {
        break;
    }
    case cmd_AudioData_request:
    {
        msg_ct++;
        //std::cout << " --- " <<   (unsigned int)msg->cmd << "\t" << (unsigned int)msg->nmb << "\t" << msg->sz << std::endl;
        uint nmb=(unsigned int)msg->nmb;
        //std::cout << "===" << nmb << std::endl;

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
            sensor_sending.store(true);
            if(msg_ct==300)
            {
                //std::cout << "=======cmd_audioData_response========" << std::endl;
                COMMAND cmd;
                cmd.cmd=cmd_AudioData_response;
                sendMsg(cmd);
                msg_ct=0;
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

void SensorThread::ontimeout()
{
    //std::cout << "---" << std::endl;
}

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
            if( (i+msgp->sz+sizeof (MESSAGE)) <= tba.size() )
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


bool SensorThread::getResponse(awl::ByteArray& resp, uint8_t to)
{
    int ct=0;
    bool result=false;
    //std::cout << "getResponse    " << msgState.load() << std::endl;
    while(ct < to)
    {
        if(msgState.load()==ms_responded)
        {
            resp = awl::Core::byteArrayRight(message,sizeof (MESSAGE));
            result = true;
            break;
        }
        ct++;
        usleep(100000);
    }
    msgState.store(ms_none);
    return result;
}


unsigned SensorThread::getMsgState()
{
    return msgState.load();
}
