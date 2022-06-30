#ifndef SENSTELNET_H
#define SENSTELNET_H

#include "net.h"
#include "sensorthread.h"

class TelnetSensorProcess : public awl::Core::VectorProcessing<SensorThread*>
{
    void prepare() override;
    void processItem(SensorThread *) override;
    void finish() override;
};

class SensTelnet : public awl::Net::TelnetThread
{
public:
    SensTelnet();
    ~SensTelnet();

    void onmessage() override;

    awl::Net::TcpServer<SensorThread>* sensorserver;
    //std::vector<SensorThread*> sensors;

private:
    void parseCommand(std::string cmd);

    std::vector<std::string> cmdv;

    TelnetSensorProcess telnet_sensorprocess;

};

#endif // SENSTELNET_H
