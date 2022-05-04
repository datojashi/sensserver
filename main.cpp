#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

#include <chrono>
#include <ctime>

#include<signal.h>
#include<unistd.h>

#include "senstelnet.h"
#include "sensorthread.h"
#include "streamthread.h"

using namespace std;

void sig_handler(int signum){
  switch(signum)
  {
   case SIGUSR1:
      break;
  case SIGUSR2:
     break;
  case SIGTERM:
     break;
  case SIGKILL:
     break;
  }

  std::cout << signum << std::endl;
}

int main()
{

    //awl::Core::test();

    awl::Core::Config config("/home/dato/server.conf");

    config.readconfig();

    /*
    std:: string addr;
    int tcpport;
    int udpport;
    addr=config.getValue("addr");
    try
    {
        tcpport=std::stoi(config.getValue("tcpport"));
        udpport=std::stoi(config.getValue("udpport"));
    }
    catch ( const std::out_of_range &e)
    {
        cout << e.what() << endl;
    }
    catch( ... )
    {
        cout << "error read config" << endl;
    }
    */



    awl::Net::SockAddr senssockaddress( config.getValue("sensaddr"),std::stoi(config.getValue("sensport")));
    awl::Net::SockAddr telnetsockaddress( config.getValue("telnetaddr"),std::stoi(config.getValue("telnetport")));


    std::cout << "sensor address=" << senssockaddress.addr()  << ':' << senssockaddress.addr() << std::endl;
    std::cout << "telnet address=" << telnetsockaddress.addr() << ':' << telnetsockaddress.port() <<  std::endl;



    awl::Net::TcpServer<SensTelnet> telnet(telnetsockaddress);
    telnet.config=&config;
    if(telnet.get_socketstate()==awl::Net::ssListening)
    {

        telnet.start(true);
    }



    //*
    awl::Net::TcpServer<SensorThread> tcpserver(senssockaddress);

    std::cout << tcpserver.get_socketstate() << endl;

    if(tcpserver.get_socketstate()==awl::Net::ssListening)
        tcpserver.start(true);
    //*/



    signal(SIGUSR1,sig_handler);
    signal(SIGUSR2,sig_handler);
    signal(SIGUSR1,sig_handler);
    signal(SIGTERM,sig_handler);
    signal(SIGKILL,sig_handler);
    while(1)
    {
       std::vector<SensorThread*> sensors = tcpserver.getConnections();
       std::vector<SensTelnet*> telnets = telnet.getConnections();

       for(size_t i=0; i<telnets.size(); i++)
       {
            telnets[i]->sensors=sensors;
       }

       std::cout << "Sensors connected: " << sensors.size() << std::endl;

       sleep(2);
    }

    return 0;
}
