#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include "net.h"
#include <cstdio>
#include <queue>
#include "audiothread.h"

#define MAX_AUDIO_CHANNELS 2

uint8_t const cypher[16] = {0x5b,0x7f,0xa1,0x59,0xbf,0xf5,0x46,0xaf,0xda,0x4c,0xc3,0x25,0xa2,0xe5,0xee,0x73};

enum  CMD
{
    cmd_ping_request    =   0x01U, // server <--> sensor
    cmd_ping_response   =   0x02U, // sensor <--> server

    cmd_AudioData_request   =   0x03U, // sensor --> server
    cmd_AudioData_response  =   0x04U, // server --> sensor

    cmd_startAudio_request   =   0x05U, // server --> sensor
    cmd_startAudio_response  =   0x06U, // sensor --> server

    cmd_stopAudio_request   =   0x07U, //  server --> sensor
    cmd_stopAudio_response  =   0x08U, //  sensor --> server

    cmd_startLive_request   =   0x09U, // server --> sensor
    cmd_startLive_response   =   0x0aU, // sensor --> server

    cmd_stopLive_request   =   0x0bU, // server --> sensor
    cmd_stopLive_response   =   0x0cU, // sensor --> server

    cmd_setRTC_request	= 0x0dU,	//server --> sensor
    cmd_setRTC_response	= 0x0eU,	//sensor --> server

    cmd_None    =0x00ffU
};

enum MSG_STATE
{
    ms_none = 0,
    ms_sent = 1,
    ms_responded=2,
};

struct __attribute__((__packed__)) COMMAND
{
    CMD cmd;
    time_t start;
    time_t stop;
};

struct __attribute__((__packed__)) MESSAGE
{
    uint16_t tag;
    uint8_t nmb;
    uint8_t cmd;
    uint32_t sz;
};


struct AudioChannel
{
    AudioChannel(int i, std::string p)
    {
        thread = new AudioThread(i,p);
        thread->start(true);
    }

    ~AudioChannel()
    {
        thread->stop();
        thread->wait();
        delete thread;
        std::cout << "Thread deleted" << std::endl;
    }

    void saveData()
    {
        thread->appendData(data);
        data.clear();
    }

    awl::ByteArray data;
    AudioThread* thread;
};

class SensorThread : public awl::Net::TcpServerThread
{
public:
    SensorThread();
    ~SensorThread();

    bool addCommand(COMMAND cmd, bool mode=true);
    bool getResponse(awl::ByteArray& resp, uint8_t to=30);
    unsigned int getMsgState();


    std::atomic_bool sensor_initialised=false;
    std::atomic_uint no_pong=0;

protected:
    void getmessage();
    void onstart();
    void onwork();
    void onmessage();
    void ontimeout();


private:
    int ct;
    int ct2;
    int ct3;

    //    uint32_t cmd;
    //    uint32_t nmb;
    //    uint32_t msgsize;


    std::queue<COMMAND> commands;

    uint32_t size;
    awl::ByteArray chunk;
    AudioChannel* audiochannels[MAX_AUDIO_CHANNELS];

    int msg_ct = 0;
    uint8_t send_ct {0};

    char reqdata[256];

    void processAudioData();
    void processCommand();


    void sendMsg(CMD cmd);


    std::atomic_uint msgState=ms_none;
    uint8_t waitmsg;


};

#endif // SENSORTHREAD_H
