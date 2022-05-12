#include <mqueue.h>

#include "MQRequestChannel.h"

using namespace std;


// TODO: implement MQRequestChannel constructor/destructor and functions
MQRequestChannel::MQRequestChannel(const std::string &_name, const Side _side, int _msg_size) : RequestChannel(_name, _side) , msg_size(_msg_size){
    string mq_name1 = "mq_" + my_name + "_1";
	string mq_name2 = "mq_" + my_name + "_2";
    if (my_side == CLIENT_SIDE) {
		readMQD = open_MQR(mq_name1, O_RDONLY);
		writeMQD = open_MQR(mq_name2, O_WRONLY);
	}
	else {
		writeMQD = open_MQR(mq_name1, O_WRONLY);
		readMQD = open_MQR(mq_name2, O_RDONLY);
	}
}

MQRequestChannel::~MQRequestChannel () {
	// close(rfd);
	// close(wfd);
	
	// remove(fifo_name1.c_str());
	// remove(fifo_name2.c_str());
    string mq_name1 = "mq_" + my_name + "_1";
	string mq_name2 = "mq_" + my_name + "_2";
    if (my_side == SERVER_SIDE) {
        
        if(mq_unlink(("/" +mq_name1).c_str()) == -1){
            cout << "/" + mq_name1 << endl;
            perror("mq unlink for name 1");
            // exit(EXIT_FAILURE);

        }
        if(mq_unlink(("/" +mq_name2).c_str()) == -1){
            cout << "/" + mq_name2 << endl;
            perror("mq unlink for name 2");
            // exit(EXIT_FAILURE);

        }

        if(mq_close(readMQD) == -1){
            perror("mq close for read");
            exit(EXIT_FAILURE);

        }
        if(mq_close(writeMQD) == -1){
            perror("mq close for write");
            exit(EXIT_FAILURE);

        }
    }
}

mqd_t MQRequestChannel::open_MQR(std::string _name, int _mode){
    mq_attr attr{};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = msg_size;
    // mqd_t open = mq_open(("/"+ _name).c_str(), _mode | O_CREAT,0600, &attr);
    // if(open == -1){
    //     perror("mq open from MQR");
    //     exit(EXIT_FAILURE);

    // }
    mqd_t open = mq_open(("/"+ _name).c_str(), _mode | O_CREAT,0600, &attr);
    if(open == -1){
        perror("mq open from MQR");
        exit(EXIT_FAILURE);

    }
    return open;

}

int MQRequestChannel::cread (void* msgbuf, int msgsize) {
    (void) msgsize;
    return mq_receive(readMQD, (char*) msgbuf, 8192, NULL); 
	// return msg_size;
}

int MQRequestChannel::cwrite (void* msgbuf, int msgsize) {
	return mq_send(writeMQD, (char*) msgbuf, msgsize, 0);
}