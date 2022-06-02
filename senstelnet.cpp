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
                std::string s = "\r\n --- Sensor: " + std::to_string(j)+"\r\n";
                std::cout << "\t--- Sensor: " << j << std::endl;
                socket->send(s);
                sendPrompt();

            }
        }
        else if(cmdv.at(0)=="get")
        {



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
                    cmd.cmd=cmd_setRTC_request;
                    sensors.at(s_nmb)->addCommand(cmd);
                }

            }
        }
        else if(cmdv.at(0)=="start")
        {
            if(cmdv.size() == 2)
            {
                uint s_nmb=std::stoi(cmdv.at(1));

                if(sensors.size() > s_nmb)
                {
                    COMMAND cmd;
                    cmd.cmd=cmd_startAudio_request;
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
                    sendPrompt();
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
                    cmd.cmd=cmd_stopAudio_request;
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
                    sendPrompt();
                }

            }
        }
        else
        {

        }



    }

}

void SensTelnet::onmessage()
{
    std::string s = awl::Core::byteArrayToString(message,0,message.size());
    parseCommand(s);
    //std::cout << s  << std::endl;
}

