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
#include "appthread.h"

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



class MainSensorProcess : public awl::Core::VectorProcessing<SensorThread*>
{
public:
    void prepare(){

    }

    void processItem(SensorThread* sens)
    {
        if(!sens->sensor_initialised.load())
        {
            COMMAND cmd;
            cmd.cmd=cmd_setConfig_request;
            sens->addCommand(cmd);
            awl::ByteArray response;
            std::cout << "*************************" << std::endl;
            if(sens->getResponse(response))
            {
                awl::Core::printhex(response);
                sens->sensor_initialised.store(true);
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
                            sens->sensTime=awl::Core::stringToTimeStamp(s,"%S.%M.%H.%d.%m.%y",ok);
                            std::cout << "Reconecting - " << s  << '\t' << sens->sensTime << std::endl;
                            uint32_t start_sector=*((uint32_t*)(response.data()+12));
                            uint32_t stop_sector=*((uint32_t*)(response.data()+16));
                            sens->getTimeBySector(start_sector);
                            cmd.t1=sens->getTimeBySector(stop_sector);
                        }
                    }
                    sens->setLastCommand(cmd);
                    sens->sensor_sending.store(true);
                }
                std::cout << "Sensor   RTC Set OK" << std::endl;
            }
            else
            {
                std::cout << "Sensor    RTC Set Error! No Response from sensor." << std::endl;
            }
        }
        else
        {
            COMMAND cmd;
            cmd.cmd=cmd_ping_request;
            if(!sens->sensor_sending.load())
            {
                sens->addCommand(cmd,false);
                awl::ByteArray response;
                if(!sens->getResponse(response))
                {
                    std::cout << "Sensor  No response on ping" << std::endl;
                    if(sens->no_pong.load()==2)
                    {
                        sens->no_pong.store(0);
                        sens->stop();
                    }
                    else
                        sens->no_pong.fetch_add(1);
                }
                else
                {
                    std::cout << "Sensor  response on ping OK" << std::endl;
                }
            }
            else
                sens->sensor_sending.store(false);
        }
    }

    void finish()
    {

    }
}main_sensorprocess;

int main()
{

    //awl::Core::test();

    awl::Core::Config config("/home/dato/server.conf");

    config.readconfig();


    awl::Net::SockAddr senssockaddress( config.getValue("sensaddr"),std::stoi(config.getValue("sensport")));
    awl::Net::SockAddr telnetsockaddress( config.getValue("telnetaddr"),std::stoi(config.getValue("telnetport")));


    awl::Net::SockAddr cliaddress("127.0.0.1",19899);
    awl::Net::SockAddr appaddress("127.0.0.1",19000);
    AppThread appcli(cliaddress,appaddress);
    appcli.start(true);

    std::cout << "sensor address=" << senssockaddress.addr()  << ':' << senssockaddress.port() << std::endl;
    std::cout << "telnet address=" << telnetsockaddress.addr() << ':' << telnetsockaddress.port() <<  std::endl;


    awl::Net::TcpServer<SensTelnet> telnetserver(telnetsockaddress);
    telnetserver.config=&config;
    if(telnetserver.get_socketstate()==awl::Net::ssListening)
    {
        telnetserver.start(true);
    }

    std::string synctime_s = config.getValue("synctime");


    awl::Net::TcpServer<SensorThread> sensorserver(senssockaddress);

    std::cout << sensorserver.get_socketstate() << endl;

    if(sensorserver.get_socketstate()==awl::Net::ssListening)
        sensorserver.start(true);


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

    //std::vector<SensorThread*>* sensors;
    std::vector<SensTelnet*>* telnets;

    while(1)
    {
        telnets = telnetserver.lock();
        for(size_t i=0; i<telnets->size(); i++)
        {
            (*telnets)[i]->sensorserver=&sensorserver;
        }
        telnetserver.unlock();

        if( time(0) > synctime_t )
        {
            //todo
            // Build and add to sensorthreads cmd_startAudio_request cmd
            awl::Core::awl_addDay(synctime_t);
            syncTime = awl::Core::timeToStringt(synctime_t);
            syncDate = awl::Core::dateToStringt(synctime_t);

            std::cout << "Next syncing at: " << syncTime << " " << syncDate << std::endl;
        }

        sensorserver.processConnections(&main_sensorprocess);
        awl::Core::awl_delay_ms(1000);
    }

/*
    while(1)
    {

        sensors = sensorserver.lock();
        if(sensors==nullptr)
        {
            awl::Core::awl_delay_ms(100);
            std::cout << "Mutex lock ERROR" << std::endl;
            continue;
        }
        telnets = telnet.getConnections();

        appcli.sensors=sensors;

        for(size_t i=0; i<telnets.size(); i++)
        {
            telnets[i]->server=&sensorserver;
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

        for(size_t i=0; i<sensors->size(); i++)
        {
            if(!sensors->at(i)->sensor_initialised.load())
            {
                COMMAND cmd;
                cmd.cmd=cmd_setConfig_request;
                sensors->at(i)->addCommand(cmd);
                awl::ByteArray response;
                std::cout << "*************************" << std::endl;
                if(sensors->at(i)->getResponse(response))
                {
                    awl::Core::printhex(response);
                    sensors->at(i)->sensor_initialised.store(true);
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
                                sensors->at(i)->sensTime=awl::Core::stringToTimeStamp(s,"%S.%M.%H.%d.%m.%y",ok);
                                std::cout << "Reconecting - " << s  << '\t' << sensors->at(i)->sensTime << std::endl;
                                uint32_t start_sector=*((uint32_t*)(response.data()+12));
                                uint32_t stop_sector=*((uint32_t*)(response.data()+16));
                                cmd.t0=sensors->at(i)->getTimeBySector(start_sector);
                                cmd.t1=sensors->at(i)->getTimeBySector(stop_sector);
                            }
                        }
                        sensors->at(i)->setLastCommand(cmd);
                        sensors->at(i)->sensor_sending.store(true);
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
            for(size_t i=0; i<sensors->size(); i++)
            {

                if(!sensors->at(i)->sensor_sending.load())
                {
                    sensors->at(i)->addCommand(cmd,false);
                    awl::ByteArray response;
                    if(!sensors->at(i)->getResponse(response))
                    {
                        std::cout << "Sensor " << i << "  No response on ping" << std::endl;
                        if(sensors->at(i)->no_pong.load()==2)
                        {
                            sensors->at(i)->no_pong.store(0);
                            sensors->at(i)->stop();
                        }
                        else
                            sensors->at(i)->no_pong.fetch_add(1);
                    }
                    else
                    {
                        std::cout << "Sensor " << i << "  response on ping OK" << std::endl;
                    }
                }
                else
                    sensors->at(i)->sensor_sending.store(false);

            }
            t=_t;
        }

        //tcpserver.clearConnetcions();
        sensorserver.unlock();
        sleep(2);
    }
//*/
    return 0;
}
