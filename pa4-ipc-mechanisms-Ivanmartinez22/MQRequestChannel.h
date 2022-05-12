#ifndef _MQREQUESTCHANNEL_H_
#define _MQREQUESTCHANNEL_H_

#include "RequestChannel.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 #include <mqueue.h>
 using namespace std;


class MQRequestChannel : public RequestChannel {
// TODO: declare derived components of MQRequestChannel from RequestChannel

private:
    
	int msg_size;
    mqd_t writeMQD;
    mqd_t readMQD;
    string write;
    string read;
    mqd_t open_MQR(std::string _name, int _mode);
	

public:
	MQRequestChannel (const std::string &_name, const Side _side, int _msg_size);
	~MQRequestChannel ();
    int cread (void* msgbuf, int msgsize);
	int cwrite (void* msgbuf, int msgsize);

	
};

#endif
