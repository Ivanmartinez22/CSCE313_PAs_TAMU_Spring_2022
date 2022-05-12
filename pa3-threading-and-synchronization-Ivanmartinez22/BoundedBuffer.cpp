#include "BoundedBuffer.h"

using namespace std;

// BoundedBuffer::BoundedBuffer (int _cap, string name) : cap(_cap), name(name) {
//     // modify as needed'
//     slot = true;
//     data = false;

// }

BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed'


}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}


void BoundedBuffer::push (char* msg, int size) {
    // cout << "push on " << this->name << endl;
    // cout << q.size() << endl;
    vector<char> char_vec_loc;
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    for(int i = 0; i < size; i++){
        char_vec_loc.push_back(msg[i]);
    }
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck,  [this]{return (int(q.size()) < (cap + 1));});
    
    // 3. Then push the vector at the end of the queue
    q.push(char_vec_loc);
    // 4. Wake up threads that were waiting for push
    lck.unlock();
    cv.notify_all();
}

int BoundedBuffer::pop (char* &msg, int size) {
    // 1. Wait until the queue has at least 1 item
    // cout << "pop off " << this->name << endl;
    // cout << q.size() << endl;
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [this]{return (int(q.size()) >= 1);});
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    std::vector<char> char_vec_loc = q.front();
    q.pop();
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    assert(int(char_vec_loc.size()) <= size);
    
    memcpy(msg, char_vec_loc.data(), int(char_vec_loc.size()));//????????????????
    // 4. Wake up threads that were waiting for pop
    lck.unlock();
    cv.notify_all();
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    return int(char_vec_loc.size());
}

bool BoundedBuffer::slot_function() {
    return (int(q.size()) < (cap + 1));
}

bool BoundedBuffer::data_function() {
    return (int(q.size()) >= 1);
}

size_t BoundedBuffer::size () {
    return q.size();
}
