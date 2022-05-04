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
    for(uint i=0; i<cmdv.size(); i++)
    {
        std::cout << cmdv.at(i) << std::endl;
    }

}

void SensTelnet::onmessage()
{
    std::string s = awl::Core::byteArrayToString(message,0,message.size());
    parseCommand(s);
    //std::cout << s  << std::endl;
}
