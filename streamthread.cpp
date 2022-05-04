#include "streamthread.h"

StreamThread::StreamThread(awl::Net::SockAddr addr) : awl::Net::UdpServerThread(addr)
{

}

void StreamThread::onstart()
{
   std::cout << "Stream Thread started" << std::endl;
}

void StreamThread::getmessage()
{
    //std::string s=std::string(tba.data());
    //std::cout << s << '\t' << peer.addr() << std::endl;
    //std::cout << s <<  std::endl;
    awl::Core::printhex(tba.data(),36);
}
