#include "senstelnet.h"

SensTelnet::SensTelnet()
{

}

SensTelnet::~SensTelnet()
{

}


void SensTelnet::parseCommand(std::string cmd)
{
    cmdv.clear();

    awl::Core::awl_split(cmd,' ',cmdv);

    std::cout << cmdv.size() << std::endl;
    if(cmdv.size() > 0)
    {
        std::cout << cmdv.at(0) << std::endl;

        if(cmdv.at(0)=="list")
        {
            std:: cout << "Connected sensors " << sensors.size() << std::endl;
            for(size_t j=0; j<sensors.size(); j++)
            {
                std::string mode="";
                COMMAND cmd=sensors.at(j)->getLastCommand();

                std::cout << "**********" << cmd.cmd << std::endl;

                switch(cmd.cmd)
                {
                case cmd_startTransmit_request:
                {
                    if(cmd.live)
                    {
                        mode="Live";
                    }
                    else
                    {
                        mode="Sending data\t"
                                +awl::Core::dateToStringt(cmd.t0)+"_"
                                +awl::Core::timeToStringt(cmd.t0)+"\t"
                                +awl::Core::dateToStringt(cmd.t1)+"_"
                                +awl::Core::timeToStringt(cmd.t1);

                        std::cout << "========= 1 Mode " << mode << std::endl;
                    }
                }
                case cmd_stopTransmit_request:
                {
                    break;
                }
                default:
                    break;
                }
                std::string s = "\t"
                        + std::to_string(j)+"\t"
                        +awl::Core::dateToStringt(sensors.at(j)->sensTime)+"_"
                        +awl::Core::timeToStringt(sensors.at(j)->sensTime)+"\t"
                        +mode
                        + "\r\n";
                std::cout << "\t--- Sensor: " << j << std::endl;
                socket->send(s);
            }
        }
        else if(cmdv.at(0)=="setrtc")
        {
            if(cmdv.size() == 2)
            {
                uint s_nmb=std::stoi(cmdv.at(1));

                std::cout << "setrtc << " << s_nmb << std::endl;

                if(sensors.size() > s_nmb)
                {
                    COMMAND cmd;
                    cmd.cmd=cmd_setConfig_request;
                    sensors.at(s_nmb)->addCommand(cmd);
                }

            }
        }
        else if(cmdv.at(0)=="start")
        {
            if(cmdv.size() >= 2 && cmdv.size()<=5)
            {
                uint s_nmb=std::stoi(cmdv.at(1));
                if(sensors.size() > s_nmb)
                {
                    COMMAND cmd;
                    cmd.cmd=cmd_startTransmit_request;
                    cmd.t0=0;
                    cmd.t1=0;
                    cmd.live=false;
                    if(cmdv.size()==4)
                    {
                        bool ok;

                        time_t t0 = awl::Core::stringToTimeStamp(cmdv.at(2),"%d.%m.%y_%H:%M:%S",ok);
                        if(!ok)
                        {
                            std::cout << "Invalid argument " << cmdv.at(2) << std::endl;
                        }
                        time_t t1 = awl::Core::stringToTimeStamp(cmdv.at(3),"%d.%m.%y_%H:%M:%S",ok);
                        if(!ok)
                        {
                            std::cout << "Invalid argument " << cmdv.at(3) << std::endl;
                        }
                        std::cout << "******************** " <<  cmd.t0 << '\t' << cmd.t1 << std::endl;
                        if(t0 < t1)
                        {
                            cmd.t0=t0;
                            cmd.t1=t1;
                        }
                        else
                        {
                            std::cout << "Invalid arguments " << cmdv.at(2)  << '\t' << cmdv.at(3) << std::endl;
                        }
                    }
                    else if(cmdv.size()==3)
                    {
                        bool ok;
                        time_t t1 = awl::Core::stringToTimeStamp(cmdv.at(2),"%d.%m.%y_%H:%M:%S",ok);
                        if(!ok)
                        {
                            std::cout << "Invalid argument " << cmdv.at(2) << std::endl;
                        }
                        cmd.t1=t1;
                    }
                    else if(cmdv.size()==2)
                    {
                        cmd.live=true;
                    }
                    sensors.at(s_nmb)->addCommand(cmd);
                    awl::ByteArray resp;
                    std::string s;
                    if(sensors.at(s_nmb)->getResponse(resp,100))
                    {
                        s="\r\nOK\r\n";
                    }
                    else
                    {
                        s="\r\nError, no response\r\n";
                    }
                    socket->send(s);

                }
            }
        }
        else if(cmdv.at(0)=="stop")
        {
            if(cmdv.size() == 2)
            {
                uint s_nmb=std::stoi(cmdv.at(1));

                if(sensors.size() > s_nmb)
                {
                    COMMAND cmd;
                    cmd.cmd=cmd_stopTransmit_request;
                    sensors.at(s_nmb)->addCommand(cmd);
                    awl::ByteArray resp;
                    std::string s;
                    if(sensors.at(s_nmb)->getResponse(resp,100))
                    {
                        s="\r\nOK\r\n";
                    }
                    else
                    {
                        s="\r\nError, no response\r\n";
                    }
                    socket->send(s);
                }
            }
        }
        else
        {

        }
        sendPrompt();
    }
}

void SensTelnet::onmessage()
{
    std::string s = awl::Core::byteArrayToString(message,0,message.size());
    parseCommand(s);
    //std::cout << s  << std::endl;
}

