#ifndef SENSTELNET_H
#define SENSTELNET_H

#include "net.h"
#include "sensorthread.h"

class SensTelnet : public awl::Net::TelnetThread
{
public:
    SensTelnet();
    ~SensTelnet();

    void onmessage() override;

    std::vector<SensorThread*> sensors;

private:
    void parseCommand(std::string cmd);

    std::vector<std::string> cmdv;

};

#endif // SENSTELNET_H
