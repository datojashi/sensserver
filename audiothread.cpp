#include "audiothread.h"

AudioThread::AudioThread(int i, std::string fpath) : tag(i)
{
    fn=fpath+"/"+std::to_string(i);
    f.open(fn,std::ios_base::out | std::ios_base::app);
    std::cout << "Open file: " << fn << std::endl;
    data_in=&data1;
    data_out=&data2;
    ct=0;
}

AudioThread::~AudioThread()
{

}


void AudioThread::aLawDecode(awl::ByteArray buf, int16_t* buf16, int sz)
{
    uint8_t alawbyte;
    uint8_t chord=0, step=0;
    int sign=0;
    int16_t sample=0;
    uint16_t shifts[8][2]={{1,1},{1,1},{2,2},{3,4},{4,8},{5,16},{6,32},{7,64}};

    for(int i=0; i<sz; i++)
    {
        alawbyte=buf[i];
        sign = (alawbyte & 0x80);
        chord=(alawbyte&0x70) >> 4;
        step=alawbyte&0x0f;
        sample=(step << shifts[chord][0]) | shifts[chord][1];
        if(chord > 0)
        {
            sample=sample|(1 << (shifts[chord][0]+4));
        }
        if(sign!=0)
        {
            sample=-sample;
        }
        buf16[i]=sample*8;
    }

}

void AudioThread::appendData(awl::ByteArray d)
{
    mutex.lock();
    data_in->push_back(d);
    mutex.unlock();

}

void AudioThread::onstart()
{
    std::cout << "Started Audio thread " << tag << std::endl;
}

void AudioThread::onstop()
{
    std::cout << "Stoped Audio thread " << tag << std::endl;
}

void AudioThread::run()
{

    mutex.lock();
    if(data_in->size() > 0)
    {
        std::vector<awl::ByteArray>* tp=data_in;
        data_in=data_out;
        data_out=tp;
    }
    mutex.unlock();



    for(unsigned int i=0; i<data_out->size(); i++)
    {
        //int16_t buf16[data_out->at(i).size()];
        //aLawDecode(data_out->at(i),buf16,data_out->at(i).size());
        //awl::ByteArray ba;
        //awl::Core::initba(ba,(char*)buf16,data_out->at(i).size()*2);
        //f.write(ba);

        f.write(data_out->at(i));
    }
    data_out->clear();

}
