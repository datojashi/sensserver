#ifndef STEAMTHREAD_H
#define STEAMTHREAD_H

#include "net.h"

class StreamThread : public awl::Net::UdpServerThread
{
public:
    StreamThread(awl::Net::SockAddr);

    void getmessage();
    void onstart();

};

#endif // STEAMTHREAD_H
