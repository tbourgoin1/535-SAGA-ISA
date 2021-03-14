#include <iostream>
#include <string>
#include <windows.h> //for Sleep
using namespace std;

string* ram[64] = {0}; // we want a 4:1 mapping from cache to DRAM - just want minimum DRAM space for demo for now. tag + index + dirty = 7 bits. data = 32 so 39 bit lines total
string* cache[16] = {0}; // 1 dirty + 1 valid + 2 tag + 4 index + 1 offset + 64 (32 x 2 words per line) data = 73 bits per line x 16 lines for demo = 1168 bit cache
int cycles = 0;

// our choice of write policy - we have to do write-through allocate since we have cache and dram. main memory less than address space
// need logic for valid/invalid, clean/dirty cache elements
// cache 0 delay, main memory 3 delay (3 seconds, ms or somewhere between?)
// all memory timing and commands related stuff - slides
// keep a count of each W and R command - clock cycle representation in cache vs dram


// command line interface (DONE)
// cache at least "16 lines" (DONE)
// need initialization function to set all array elems to 0 - cache lines start invalid and clean (DONE)

int write(string addr, string data){ //respond with "wait" or "done", write to mem or cache
    if(addr.size() != 7 || data.size() != 32){
        cout << "incorrect parameter format passed to write. Try again!" << endl;
        return 0;
    }
    //parse input
    string dirty, valid; // cache only, need to get from the addr passed in by searching cache
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    return 0;
}

int read(string addr){ //respond with "wait" or "done" and return stored value
    if(addr.size() != 7){
        cout << "incorrect parameter format passed to read. Try again!" << endl;
        return 0;
    }
    //parse input
    string dirty, valid; // cache only, need to get from the addr passed in by searching cache
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    return 0;
}

// addr is the address we want, memLvl tells us cache or dram (1 or 0, respectively)
int view(string addr, string memLvl){ //prints the tag, index, and offset along with the data they map to - if level is 1 for cache then also valid and dirty bits
    if(addr.size() != 7 || memLvl.size() != 1){
        cout << "incorrect parameter format passed to view. Try again!" << endl;
        return 0;
    }
    //parse input
    string dirty, valid; // cache only, need to get from the addr passed in by searching cache
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    return 0;
}

int main(){
    while(1){
        string command, param1, param2; // command for w, r, or v, 1st parameter for command, 2nd parameter for command, not present with r
        cin >> command; //read in command and test
        if(command == "w"){
            cin >> param1;
            cin >> param2;
            cout << "write returned: \n" << write(param1, param2) << endl;
        }
        else if(command == "r"){
            cin >> param1;
            cout << "read returned: \n" << read(param1) << endl;
        }
        else if(command == "v"){
            cin >> param1;
            cin >> param2;
            cout << "view returned: \n" << view(param1, param2) << endl;
        }
        else if(command == "exit"){
            cout << "exiting program..." << endl;
            break;
        }
        else{
            cout << "please enter a valid input!" << endl;
        }
    }
    return 0;
}