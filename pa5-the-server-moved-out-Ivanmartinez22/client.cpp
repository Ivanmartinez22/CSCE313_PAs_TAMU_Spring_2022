// your PA3 client code here
#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>
#include <utility>      // std::pair, std::make_pair
#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "TCPRequestChannel.h"

#include <functional>
using std::ref;
using std::cref;


// ecgno to use for datamsgs
#define EGCNO 1
using std::cout;

using namespace std;

struct pair_struct{

    int p_no;
    double value;
    bool kill = false;
};

// void patient_thread_function (/* add necessary arguments */) { //starter code 
//     // functionality of the patient threads
// }

// void file_thread_function (/* add necessary arguments */) {
//     // functionality of the file thread
// }

// void worker_thread_function (/* add necessary arguments */) {
//     // functionality of the worker threads
// }

// void histogram_thread_function (/* add necessary arguments */) {
//     // functionality of the histogram threads
// }

void patient_thread_function (int p_num, int n_requests, int buffercapacity, int ecg, BoundedBuffer &request_buffer) {
    // functionality of the patient threads
    double up_by = 0.004;
    double time = 0.00;
    for(int i = 0; i < n_requests; i++){
        char * buf = new char[buffercapacity];
        datamsg y(p_num, time, ecg); // push this to request buffer 
        memcpy(buf, &y, sizeof(datamsg));
        request_buffer.push(buf, sizeof(datamsg));
        time += up_by;
        delete[] buf;
    }

}

void file_thread_function (string file_name, int buffercapacity, int filesize, BoundedBuffer &request_buffer) {
    // FILE *fp = fopen (file_name.c_str(), "w");
    // if(!fp){
    //     cout << "file not open" << endl;
    // }
    // filemsg fm(0, 0); // need length file message // push to request buffer 
     // place holder 
    int len = sizeof(filemsg) + (file_name.size() + 1);
    __int64_t num_of_messages = floor(filesize/buffercapacity); // needs one extra message but the last message is special so it is handle outside loop
    __int64_t accumulated_offset = 0;
    for(__int64_t i = 0; i < num_of_messages; i++){
        filemsg fm_loop(accumulated_offset, buffercapacity); // accumulated changes to use the correct offset // push to request buffer 
        //fwrite(buf_loop2, 1,buffercapacity,fp);
        char* buf_loop = new char[len];
        memcpy(buf_loop, &fm_loop, sizeof(filemsg)); //copying file message into buffer
		strcpy(buf_loop + sizeof(filemsg), file_name.c_str());//copying filename into buffer?
        request_buffer.push(buf_loop, len);
        accumulated_offset += buffercapacity;
        delete[] buf_loop;
    }
    // last chunk handling 
    if((filesize % buffercapacity) != 0){
        __int64_t remaining_offset = filesize - accumulated_offset; //the last offset 
        filemsg fm_last(accumulated_offset, remaining_offset); // accumulated changes to use the correct offset // push to request buffer 
        //fwrite(buf_last2, 1,remaining_offset,fp);
        char* buf_last = new char[len];
        memcpy(buf_last, &fm_last, sizeof(filemsg)); //copying file message into buffer
		strcpy(buf_last + sizeof(filemsg), file_name.c_str());//copying filename into buffer?
        request_buffer.push(buf_last, len);
        delete[] buf_last;
    }

}

void worker_thread_function (TCPRequestChannel* chan, int buffercapacity, BoundedBuffer &request_buffer, BoundedBuffer &reponse_buffer) {
    // functionality of the worker threads
    for(;;){
        //pop message from request buffer 
        char * buf = new char[buffercapacity];
        
        request_buffer.pop(buf, buffercapacity);
        


        MESSAGE_TYPE m = *((MESSAGE_TYPE*) buf);
        if (m == DATA_MSG) {
            
            datamsg* d = (datamsg*) buf;
            chan->cwrite(buf, sizeof(datamsg)); // question  data request 
            double reply;
            chan->cread(&reply, sizeof(double)); //answer pulled data
            pair_struct send;
            send.p_no = d->person;
            send.value = reply;
            //cout << "processing data message for patient :" << d->person << endl;
            //cout << "q.size :" << request_buffer.size() << endl;
            memcpy(buf, &send, sizeof(pair_struct));
            reponse_buffer.push(buf, buffercapacity);              
        }
        else if (m == FILE_MSG) {
            filemsg f = *((filemsg*) buf);
            FILE * fp;
            
            

            
	        string filename = buf + sizeof(filemsg);
            filename = "received/" + filename;
            fp = fopen ( filename.c_str() , "r+" );
            char * buf_loc = new char[buffercapacity];
            int len = sizeof(filemsg) + (filename.size() + 1);
            chan->cwrite(buf, len);
            chan->cread(buf_loc,buffercapacity);
            
            
            fseek (fp, f.offset, SEEK_SET );
            //cout << f.length << endl;
            //fputs (buf_loc, fp);
            fwrite(buf_loc, 1, f.length,fp);
            fclose(fp);
            delete[] buf_loc;
        }
        else if (m == QUIT_MSG) {
            // cout << "I quit" << endl;
            // cout << "q.size :" << request_buffer.size() << endl;
            delete[] buf;
            break;
            
        }
        delete[] buf;
    }
}

void histogram_thread_function (int buffercapacity, HistogramCollection &collection, BoundedBuffer &response_buffer) {
    // functionality of the histogram threads
    for(;;){
        //pop response from response buffer
        char * buf = new char[buffercapacity];
        
        response_buffer.pop(buf, buffercapacity);
        // HistogramCollection::update(resp->p_num, resp->double);
        pair_struct resp;
        memcpy(&resp, buf, sizeof(pair_struct));
        if(resp.kill){
            delete[] buf;
            break;
        }
        collection.update(resp.p_no, resp.value);
        delete[] buf;

        
    }
}

// void quit_thread_function (int w,BoundedBuffer &request_buffer) {
//     // functionality of the histogram threads
//     for (int i = 0; i < w; i++) { // quitting workers 
//         MESSAGE_TYPE m = QUIT_MSG;
//         request_buffer.push((char *) &m, sizeof (MESSAGE_TYPE));
//         // cout << "pushed quit " << i << endl;
//     }
// }

int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
    bool file = false;
    bool patient = false;
    bool histo = false;
	string f = "";	// name of file to be transferred
    string ip_address = "";
    string port = "";
    //vector<string> channel_names;
    vector<TCPRequestChannel*> channels;
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:a:r:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                patient = true;
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
                histo = true; 
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                file = true;
                break;
            case 'a':
				ip_address = optarg;
                break;
            case 'r':
				port = optarg;
                break;
		}
	}
    // cout << ip_address << endl;
    
	// fork and exec the server
    // int pid = fork();
    // if (pid == 0) {
    //     execl("./server", "./server", "-m", (char*) to_string(m).c_str(), "-r",(char*) port.c_str(), nullptr);
    // }
    // sleep(10);
	// initialize overhead (including the control channel)
    // cout << "control active" << endl;
	TCPRequestChannel* chan = new TCPRequestChannel(ip_address, port);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    //Creating w channels for w work threads 
    for (int i = 0; i < w; i++) {
        // MESSAGE_TYPE ncr = NEWCHANNEL_MSG;
		// cout << "sending new channel request" << endl;
    	// chan->cwrite(&ncr, sizeof(MESSAGE_TYPE));
		// char* buf_loc = new char[m];
		// chan->cread(buf_loc, sizeof(string));
        // string loc_name = buf_loc;
		// MESSAGE_TYPE m = QUIT_MSG;
    	// chan->cwrite(&m, sizeof(MESSAGE_TYPE));
        //channel_names.push_back(loc_name);
        // cout << i << endl;
		TCPRequestChannel * chan_loc = new TCPRequestChannel(ip_address, port); // channel switched to new channel
        channels.push_back(chan_loc);
		
    }

    


    /* create all threads here */
    
    thread * p_threads = new thread[p];
    thread * w_threads = new thread[w];
    thread * h_threads = new thread[h];
    thread * f_threads = new thread[1];
    //creating file thread
    
    if(file){
        FILE * fp;
        string filename = "received/" + f;
        fp = fopen ( filename.c_str() , "w+" );
        
        
        
        filemsg fm(0, 0); // need length file message  
		string fname = f; //file name from user input
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf = new char[len];
		memcpy(buf, &fm, sizeof(filemsg)); //copying file message into buffer
		strcpy(buf + sizeof(filemsg), fname.c_str());//copying filename into buffer?
		chan->cwrite(buf, len);  // I want the file length; // sending buffer to channel aka sending file message 
		// this block of code just gets the length of the file ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		__int64_t reply_int;
		chan->cread(&reply_int, sizeof(__int64_t));
		//cout << reply_int << " file size/length " << endl;
		int filesize = reply_int;
        f_threads[0] = thread(file_thread_function, f,m,filesize,ref(request_buffer));
        delete[] buf;
        fclose(fp);
    }

    //creating worker threads 
    for (int i = 0; i < w; i++) {
        TCPRequestChannel * in = channels[i];
        w_threads[i] = thread(worker_thread_function, in, m, ref(request_buffer), ref(response_buffer));
    }
    //creating patient threads 
    if(patient){
        for (int i = 0; i < p; i++) {
            p_threads[i] = thread(patient_thread_function,(i+1), n, m, 1, ref(request_buffer));
        }
    }

    

    //creating histogram threads 
    if(histo){
        for (int i = 0; i < h; i++) {
            h_threads[i] = thread(histogram_thread_function,m, ref(hc), ref(response_buffer));
        }
    }

    


	/* join all threads here */
    //joining p threads
    if(patient){
        for (int i = 0; i < p; i++) {
        
            p_threads[i].join();
        }
    }
    if(file){//join file thread if any 
        f_threads[0].join();
    }
    for (int i = 0; i < w; i++) { // quitting workers 
        MESSAGE_TYPE m = QUIT_MSG;
       request_buffer.push((char *) &m, sizeof (MESSAGE_TYPE));
       //cout << "pushed quit" << endl;
    }
    // thread quit(quit_thread_function, w, ref(request_buffer));
    // quit.join();

    for (int i = 0; i < w; i++) {//joining workers 
       
       w_threads[i].join();
    }

    for (int i = 0; i < h; i++) { // special packets 
        pair_struct killer;
        killer.p_no = 0;
        killer.value = 0;
        killer.kill = true;
       response_buffer.push((char *) &killer, sizeof (pair_struct));
    }

    if(histo){
        for (int i = 0; i < h; i++) {//joining histograms 
        
            h_threads[i].join();
        }
    }
    delete[] p_threads;delete[] w_threads;delete[] h_threads;delete[] f_threads;



	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    // quit, close and deallocate channels 
    for (int i = 0; i < w; i++) {
        MESSAGE_TYPE m = QUIT_MSG;
        channels[i]->cwrite(&m, sizeof(MESSAGE_TYPE));
		delete channels[i];
    }


	// quit and close control channel


    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

	// wait for server to exit
	wait(nullptr);
}
