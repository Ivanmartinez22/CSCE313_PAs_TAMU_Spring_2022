#include <sys/mman.h>

#include "SHMRequestChannel.h"

using namespace std;
// class SHMQueue {
// private:
//     std::string name;
//     int length;

//     char* segment;
//     sem_t* recv_done;
//     sem_t* send_done;
    
// public:
//     SHMQueue(const std::string _name, int _length);
//     ~SHMQueue();

//     int shm_receive (void* msgbuf, int msgsize);
//     int shm_send (void* msgbuf, int msgsize);
// };

// class SHMRequestChannel : public RequestChannel {
// // TODO: declare derived components of SHMRequestChannel from RequestChannel
// private:
//     SHMQueue* writeSHQ;
//     SHMQueue* readSHQ;
//     string write;
//     string read;
//     //mqd_t open_MQR(std::string _name, int _mode);
	

// public:
// 	SHMRequestChannel (const std::string &_name, const Side _side, int length_seg);
// 	~SHMRequestChannel ();
//     int cread (void* msgbuf, int msgsize);
// 	int cwrite (void* msgbuf, int msgsize);
// };

SHMQueue::SHMQueue(const string _name, int _length) : name(_name), length(_length) {
    // TODO: implement SHMQueue constructor
    int fd = shm_open(name.c_str(), O_RDWR| O_CREAT, 0600);
    if(fd == -1){
        perror("shm-queue create error");
        exit(EXIT_FAILURE);


    }
    if(ftruncate(fd, length) == -1){
        perror("ftruncate shm-queue error");
        exit(EXIT_FAILURE);
    }
    
    // recv_done = sem_open((name + "_recv").c_str(), O_CREAT, 0600, 1);
    chr_ptr = (char*)mmap(NULL, length,PROT_READ|PROT_WRITE, MAP_SHARED,fd,0);
    close(fd);
    recv_done = sem_open((name + "_recv").c_str(), O_CREAT, 0600, 1);
    if(recv_done == SEM_FAILED){
        perror("receive sem open error");
        exit(EXIT_FAILURE);
    }
    send_done = sem_open((name + "_send").c_str(), O_CREAT, 0600, 0);
    if(send_done == SEM_FAILED){
        perror("send sem open error");
        exit(EXIT_FAILURE);
    }
    
    
}

SHMQueue::~SHMQueue () {
    // TODO: implement SHMQueue destructor
    shm_unlink(name.c_str());
    sem_close(recv_done);
    sem_close(send_done);
    sem_unlink((name + "_recv").c_str());
    sem_unlink((name + "_send").c_str());
    munmap(chr_ptr, length);
    // close(fd); 
}

int SHMQueue::shm_receive (void* msgbuf, int msgsize) {
    // cout << "recieve/read " << endl;
    // TODO: implement shm_receive
    if(sem_wait(send_done) == -1){
        perror("sem recieve wait error");
        exit(EXIT_FAILURE);
    }
    // cout << "done sem wait " << endl;
    // cout << "opened shm" << endl;

    char* temp = new char[msgsize];
    
    for(int i = 0; i < msgsize; i++){
        temp[i] = chr_ptr[i];
    }
    memcpy(msgbuf,temp, msgsize);
    // read(fd,msgbuf,msgsize);
    // close(fd);
    // cout << "mapped shm" << endl;
    // cout << "unmapped" << endl;
    delete[] temp;

    
    if(sem_post(recv_done) == -1){
        perror("sem recieve wait error");
        exit(EXIT_FAILURE);
    }
    // cout << "done sem post " << endl;
    return msgsize;
}

int SHMQueue::shm_send (void* msgbuf, int msgsize) {
    // cout << "send/write" << endl;
    if(sem_wait(recv_done) == -1){
        perror("sem send wait error");
        exit(EXIT_FAILURE);
    }
    // cout << "done sem wait " << endl;
    // TODO: implement shm_send
    // cout << "opened shm" << endl;
    char* temp = new char[msgsize];
    memcpy(temp, (char*) msgbuf, msgsize);
    // close(fd);
    for(int i = 0; i < msgsize; i++){
        chr_ptr[i] = temp[i];
    }
    // cout << "mapped shm" << endl;
    
    // cout << "unmapped" << endl;
    delete[] temp;
    if(sem_post(send_done) == -1){
        perror("sem send wait error");
        exit(EXIT_FAILURE);
    }
    // cout << "done sem post " << endl;
    return msgsize;
}

// TODO: implement SHMRequestChannel constructor/destructor and functions
SHMRequestChannel::SHMRequestChannel(const std::string &_name, const Side _side, int length_seg) : RequestChannel(_name, _side) {
    string shm_name1 = "SHMQueue_" + my_name + "_1";
	string shm_name2 = "SHMQueue_" + my_name + "_2";
    if (my_side == CLIENT_SIDE) {
	// 	readMQD = open_MQR(mq_name1, O_RDONLY);
	// 	writeMQD = open_MQR(mq_name2, O_WRONLY);
        readSHQ = new SHMQueue(shm_name1, length_seg);
        writeSHQ = new SHMQueue(shm_name2, length_seg);
	}
	else {
	// 	writeMQD = open_MQR(mq_name1, O_WRONLY);
	// 	readMQD = open_MQR(mq_name2, O_RDONLY);
        writeSHQ = new SHMQueue(shm_name1, length_seg);
        readSHQ = new SHMQueue(shm_name2, length_seg);
	}
}

SHMRequestChannel::~SHMRequestChannel () {
	// close(rfd);
	// close(wfd);
	
	// remove(fifo_name1.c_str());
	// remove(fifo_name2.c_str());
    delete writeSHQ;
    delete readSHQ;
}



int SHMRequestChannel::cread (void* msgbuf, int msgsize) {
	return readSHQ->shm_receive(msgbuf, msgsize); 
}

int SHMRequestChannel::cwrite (void* msgbuf, int msgsize) {
	return writeSHQ->shm_send(msgbuf, msgsize); 
}