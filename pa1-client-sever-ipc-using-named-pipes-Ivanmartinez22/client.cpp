/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Ivan Martinez
	UIN: 529006731
	Date: 2/6/22
*/
#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;
// using namespace std::chrono;
struct Timer//Cherno's code 
{

	std::chrono::time_point<std::chrono::system_clock> start, end;
	chrono::duration<float> duration;
	Timer()
	{
		start = chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		end = chrono::high_resolution_clock::now();
		duration = end - start;
		float ms = duration.count() * 1000.0;
		cout << "timer took " << ms << "ms" << endl;

	}
};


int main (int argc, char *argv[]) {
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	bool p_request = false;
	bool t_request = false;
	bool e_request = false;
	bool new_channel_request = false;
	bool file_copy_request = false;
	int buffercapacity = MAX_MESSAGE;
	bool buffercapacity_change = false;
	// Command input handling
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				p_request = true;
				break;
			case 't':
				t = atof (optarg);
				t_request = true;
				break;
			case 'e':
				e = atoi (optarg);
				e_request = true;
				break;
			case 'f':
				filename = optarg;
				file_copy_request = true;
				break;
			case 'c':
				new_channel_request = true;
				break;
			case 'm':
				buffercapacity = atoi(optarg);
				buffercapacity_change = true;
				
				//cout << buffercapacity << endl;
				break;
		}
	}

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	//Parent and child creation 
	//  
	// Need to implent wait and proper kill handling 
	// First test case here !!!!!!!!!!!!!!!!!!!!!!!!!!!
	if(fork() != 0){
		// wait()
	}
	else {
		//cout << "test" << endl;
		if(buffercapacity_change){
			char file[] = "./server";
			char m[] = "-m";
			
			
  			char * num = new char[buffercapacity];

  			sprintf(num, "%d", buffercapacity);
			char* arg_list[] = {file,m,num,NULL};
			execvp(file,arg_list);
			delete[] num;
		}
		else{
			char file[] = "./server";
			char* arg_list[] = {file, NULL};
			execvp(file,arg_list);
		}
		
	}

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	string chan_name = "control";
     
	
	FIFORequestChannel * control = new FIFORequestChannel(chan_name, FIFORequestChannel::CLIENT_SIDE); //effectively connecting to server
	FIFORequestChannel * chan = control;
	// FIFORequestChannel * chan = chan_ptr;
	// new channel request handling ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
	
	if(new_channel_request){
		MESSAGE_TYPE ncr = NEWCHANNEL_MSG;
		// cout << "sending new channel request" << endl;
    	chan->cwrite(&ncr, sizeof(MESSAGE_TYPE));
		char* buf_loc = new char[buffercapacity];
		chan->cread(buf_loc, sizeof(string));
		chan_name = buf_loc;
		// MESSAGE_TYPE m = QUIT_MSG;
    	// chan->cwrite(&m, sizeof(MESSAGE_TYPE));
		chan = new FIFORequestChannel(chan_name, FIFORequestChannel::CLIENT_SIDE); // channel switched to new channel
		delete[] buf_loc;
	}
	//chan = FIFORequestChannel(chan_name, FIFORequestChannel::CLIENT_SIDE);

	// example data point request
	// data request handling ???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
	// single data point test cases here !!!!!!!!!!!!!!!!!!!!!!!!!!!
    if(p_request && t_request && e_request){
		char * buf = new char[buffercapacity];
		datamsg x(p, t, e);
		
		memcpy(buf, &x, sizeof(datamsg));
		chan->cwrite(buf, sizeof(datamsg)); // question  data request 
		double reply;
		chan->cread(&reply, sizeof(double)); //answer pulled data
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		delete[] buf;
	}
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	//multiple data point test cases
	if(p_request && (!t_request) && (!e_request)){
		// cout << "timer test for multiple files" << endl;
		// Timer timer;
		double up_by = 0.004;
		double time = 0.00;
		
		string filename1 = "received/x1.csv";
		ofstream myCSV(filename1);

		for(int i = 0; i < 1000; i++){
			char * buf2 = new char[buffercapacity]; 
			datamsg y(p, time, 1);
		
			memcpy(buf2, &y, sizeof(datamsg));
			chan->cwrite(buf2, sizeof(datamsg)); // question  data request 
			double reply2;
			chan->cread(&reply2, sizeof(double)); //answer pulled data

			char * buf3 = new char[buffercapacity]; 
			datamsg z(p, time, 2);
		
			memcpy(buf3, &z, sizeof(datamsg));
			chan->cwrite(buf3, sizeof(datamsg)); // question  data request 
			double reply3;
			chan->cread(&reply3, sizeof(double)); //answer pulled data
			myCSV << time << "," << reply2 << "," << reply3 << endl;
			time += up_by;
			delete[] buf2;
			delete[] buf3;

		}

		myCSV.close();
	}


    // sending a non-sense message, you need to change this !!!!!!!!!!!!!!!!!!!!!!!
	// file copying block 

	if(file_copy_request){
		
		string filename2 = "received/" + filename;
		FILE *fp = fopen (filename2.c_str(), "w");
		if(!fp){
			cout << "file not open" << endl;
		}
		filemsg fm(0, 0); // need length file message 
		string fname = filename; //file name from user input
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf = new char[len];
		memcpy(buf, &fm, sizeof(filemsg)); //copying file message into buffer
		strcpy(buf + sizeof(filemsg), fname.c_str());//copying filename into buffer?
		chan->cwrite(buf, len);  // I want the file length; // sending buffer to channel aka sending file message 
		// this block of code just gets the length of the file ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		__int64_t reply_int;
		chan->cread(&reply_int, sizeof(__int64_t));
		__int64_t filesize = reply_int;
        __int64_t num_of_messages = floor(filesize/buffercapacity); // needs one extra message but the last message is special so it is handle outside loop
        __int64_t accumulated_offset = 0;
        for(__int64_t i = 0; i < num_of_messages; i++){
            filemsg fm_loop(accumulated_offset, buffercapacity); // accumulated changes to use the correct offset
            char* buf_loop = new char[len];
            memcpy(buf_loop, &fm_loop, sizeof(filemsg)); //copying file message into buffer
            strcpy(buf_loop + sizeof(filemsg), fname.c_str());//copying filename into buffer?
            chan->cwrite(buf_loop, len);  // I want the file part!!!!!!!; // sending buffer to channel aka sending file message 
            char* buf_loop2 = new char[buffercapacity];
            chan->cread(buf_loop2,buffercapacity);
            
            fwrite(buf_loop2, 1,buffercapacity,fp);
            accumulated_offset += buffercapacity;
            delete[] buf_loop;
            delete[] buf_loop2;
        }
        // last chunk handling 
        
        if((filesize % buffercapacity) != 0){
            __int64_t remaining_offset = filesize - accumulated_offset; //the last offset 
            filemsg fm_last(accumulated_offset, remaining_offset); // accumulated changes to use the correct offset
            char* buf_last = new char[len];
            memcpy(buf_last, &fm_last, sizeof(filemsg)); //copying file message into buffer
            strcpy(buf_last + sizeof(filemsg), fname.c_str());//copying filename into buffer?
            chan->cwrite(buf_last, len);  // I want the file part!!!!!!!; // sending buffer to channel aka sending file message 
            char* buf_last2 = new char[remaining_offset];
            chan->cread(buf_last2,remaining_offset);
            fwrite(buf_last2, 1,remaining_offset,fp);
            delete[] buf_last;
            delete[] buf_last2;
        }

		delete[] buf;
		fclose(fp);
	}
	
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    if(new_channel_request){
		chan->cwrite(&m, sizeof(MESSAGE_TYPE));
		//wait(NULL);
		delete chan;
	}
	control->cwrite(&m, sizeof(MESSAGE_TYPE));
	delete control;
	wait(NULL);
	cout << "Client terminated" << endl;
}//