﻿#include <iostream>
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


    std::cout << "sensor address=" << senssockaddress.addr()  << ':' << senssockaddress.port() << std::endl;
    std::cout << "telnet address=" << telnetsockaddress.addr() << ':' << telnetsockaddress.port() <<  std::endl;



    awl::Net::TcpServer<SensTelnet> telnet(telnetsockaddress);
    telnet.config=&config;
    if(telnet.get_socketstate()==awl::Net::ssListening)
    {
        telnet.start(true);
    }

    std::string synctime_s = config.getValue("synctime");

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

    time_t t = time(0);
    time_t synctime_t=0;
    struct tm* ts = localtime(&t);






    std::string s = awl::Core::dateToStringt(1655369132,true);
    std::cout << "************** "  << s << std::endl;

    /*
    awl::ByteArray ba;
    ba.push_back(1);
    ba.push_back(2);
    ba.push_back(3);
    ba.push_back(4);
    ba.push_back(5);
    ba.push_back(77);
    size_t ofs=0;
    std::string s = "";
    for(int i=0; i<6; i++)
        s+=std::to_string(ba.at(ofs+i));
    std::cout << "&&&&&&&&&&&&&&&&&&&&   " << s << std::endl;
    */


    /*
    bool ok;
    time_t ttt=awl::Core::stringToTimeStamp("09.06.22_17:22:05","%d.%m.%y_%H:%M:%S", ok);
    if(ok)
        std::cout << "#####  " << t << '\t' <<  ttt << std::endl;
    else
        std::cout << "??????  " << t << '\t' <<  ttt << std::endl;
    //*/

    std::cout << "**** " << ts->tm_hour << std::endl;

    if(ts->tm_hour < 4)
    {
        std::string _s1 = synctime_s+" "+awl::Core::dateToStringt(t);
        synctime_t = awl::Core::stringToTimeStamp(_s1);
    }
    else
    {
        awl::Core::awl_addDay(t);
        std::string _s2 = awl::Core::dateToStringt(t)+" "+synctime_s;
        synctime_t = awl::Core::stringToTimeStamp(_s2);
    }

    std::string syncTime = awl::Core::timeToStringt(synctime_t);
    std::string syncDate = awl::Core::dateToStringt(synctime_t);

    std::cout << "Next syncing at: " << syncTime << " " << syncDate << std::endl;

    t=time(0);

    std::vector<SensorThread*> sensors; //= tcpserver.getConnections();
    std::vector<SensTelnet*> telnets; //= telnet.getConnections();

    while(1)
    {
        sensors = tcpserver.getConnections();
        telnets = telnet.getConnections();

        for(size_t i=0; i<telnets.size(); i++)
        {
            telnets[i]->sensors=sensors;
        }

        //std::cout << "Sensors connected: " << sensors.size() << std::endl;

        if( time(0) > synctime_t )
        {
            //todo
            // Build and add to sensorthreads cmd_startAudio_request cmd
            awl::Core::awl_addDay(synctime_t);
            syncTime = awl::Core::timeToStringt(synctime_t);
            syncDate = awl::Core::dateToStringt(synctime_t);

            std::cout << "Next syncing at: " << syncTime << " " << syncDate << std::endl;
        }



        for(size_t i=0; i<sensors.size(); i++)
        {
            if(!sensors.at(i)->sensor_initialised.load())
            {
                COMMAND cmd;
                cmd.cmd=cmd_setConfig_request;
                sensors.at(i)->addCommand(cmd);
                awl::ByteArray response;
                std::cout << "*************************" << std::endl;
                if(sensors.at(i)->getResponse(response))
                {
                    awl::Core::printhex(response);
                    sensors.at(i)->sensor_initialised.store(true);
                    size_t ofs=0;
                    if(response.at(ofs)==1)//reconnect
                    {
                        COMMAND cmd;
                        std::cout << "Reconecting" << std::endl;
                        if(response.at(ofs+1)==1)//transmiting
                        {
                            std::cout << "Reconecting - Sensor transmiting" << std::endl;
                            cmd.cmd=cmd_startTransmit_request;
                            if(response.at(ofs+2)==1) //live
                            {
                                cmd.live=true;
                                cmd.t0=0;
                                cmd.t0=0;
                                std::cout << "Reconecting - Sensor transmiting - Live" << std::endl;
                            }
                            else
                            {
                                cmd.live=false;
                                std::string s = "";
                                for(int i=0; i<6; i++)
                                {
                                    s+=std::to_string(response.at(ofs+3+i));
                                    if(i < 5)
                                    {
                                        s+=".";
                                    }
                                }
                                bool ok;
                                sensors.at(i)->sensTime=awl::Core::stringToTimeStamp(s,"%S.%M.%H.%d.%m.%y",ok);
                                std::cout << "Reconecting - " << s  << '\t' << sensors.at(i)->sensTime << std::endl;
                                uint32_t start_sector=*((uint32_t*)(response.data()+12));
                                uint32_t stop_sector=*((uint32_t*)(response.data()+16));
                                cmd.t0=sensors.at(i)->getTimeBySector(start_sector);
                                cmd.t1=sensors.at(i)->getTimeBySector(stop_sector);
                            }
                        }
                        sensors.at(i)->setLastCommand(cmd);
                        sensors.at(i)->sensor_sending.store(true);
                    }
                    std::cout << "Sensor " << i << "  RTC Set OK" << std::endl;
                }
                else
                {
                    std::cout << "Sensor " << i << "  RTC Set Error! No Response from sensor." << std::endl;
                }
            }
        }



        time_t _t=time(0);
        if(_t > t+5)
        {
            COMMAND cmd;
            cmd.cmd=cmd_ping_request;
            for(size_t i=0; i<sensors.size(); i++)
            {

                if(!sensors.at(i)->sensor_sending.load())
                {
                    sensors.at(i)->addCommand(cmd,false);
                    awl::ByteArray response;
                    if(!sensors.at(i)->getResponse(response))
                    {
                        std::cout << "Sensor " << i << "  No response on ping" << std::endl;
                        if(sensors.at(i)->no_pong.load()==2)
                        {
                            sensors.at(i)->no_pong.store(0);
                            sensors.at(i)->stop();
                        }
                        else
                            sensors.at(i)->no_pong.fetch_add(1);
                    }
                    else
                    {
                        std::cout << "Sensor " << i << "  response on ping OK" << std::endl;
                    }
                }
                else
                    sensors.at(i)->sensor_sending.store(false);

            }
            t=_t;
        }

        tcpserver.clearConnetcions();

        sleep(2);
    }

    return 0;
}
