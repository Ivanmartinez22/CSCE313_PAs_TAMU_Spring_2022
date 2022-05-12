#ifndef _SHMREQUESTCHANNEL_H_
#define _SHMREQUESTCHANNEL_H_

#include <semaphore.h>

#include "RequestChannel.h"


class SHMQueue {
private:
    std::string name;
    int length;
    // int fd;
    char* segment;
    sem_t* recv_done;
    sem_t* send_done;
    char* chr_ptr;
    // int fd;
    
public:
    SHMQueue(const std::string _name, int _length);
    ~SHMQueue();

    int shm_receive (void* msgbuf, int msgsize);
    int shm_send (void* msgbuf, int msgsize);
};

class SHMRequestChannel : public RequestChannel {
// TODO: declare derived components of SHMRequestChannel from RequestChannel
private:
    SHMQueue* writeSHQ;
    SHMQueue* readSHQ;
    
    // string write;
    // string read;
    //mqd_t open_MQR(std::string _name, int _mode);
	

public:
	SHMRequestChannel (const std::string &_name, const Side _side, int length_seg);
	~SHMRequestChannel ();
    int cread (void* msgbuf, int msgsize);
	int cwrite (void* msgbuf, int msgsize);
};

#endif
