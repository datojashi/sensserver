#include "appthread.h"

AppThread::AppThread(awl::Net::SockAddr _addr, awl::Net::SockAddr _peeraddr) : cli(_addr, _peeraddr)
{
    std::cout << "Appthread COn" << std::endl;
}

AppThread::~AppThread()
{

}

void AppThread::onstart()
{
    std::cout << "Appserver client thread started " << cli.socket.get_socketstate() << std::endl;
}


void AppThread::onstop()
{

}

void AppThread::run()
{
    std::string s="Hi from Sensor server";
    awl::ByteArray ba;
    awl::Core::stringToBayteArray(s,ba);
    cli.socket.send(ba);
    awl::Core::awl_delay_ms(1000);
}
