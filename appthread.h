#ifndef APPTHREAD_H
#define APPTHREAD_H

#include <iostream>
#include "net.h"

#include "sensorthread.h"

class AppThread : public awl::Core::Thread
{
public:
    AppThread(awl::Net::SockAddr _addr, awl::Net::SockAddr _peeraddr);
    ~AppThread();

    std::vector<SensorThread*>* sensors;

private:
    void onstart();
    void onstop();
    void run();

    awl::Net::TcpClient cli;

};

#endif // APPTHREAD_H
