#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include "core.h"



class AudioThread : public awl::Core::Thread
{
public:
    AudioThread(int i, std::string fpath);
    ~AudioThread();

    void run();

    void onstart();
    void onstop();

    void appendData(awl::ByteArray d);


private:
    std::vector<awl::ByteArray> data1;
    std::vector<awl::ByteArray> data2;

    std::vector<awl::ByteArray>* data_in;
    std::vector<awl::ByteArray>* data_out;


    awl::Core::File f;
    int tag;
    std::string fn;
    int ct;

    void aLawDecode(awl::ByteArray buf, int16_t* buf16, int sz);

};

#endif // AUDIOTHREAD_H
