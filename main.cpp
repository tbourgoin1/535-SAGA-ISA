#include <iostream>
#include <string>
using namespace std;

int* ram = new int[2496]; // we want a 4:1 mapping from cache to DRAM - just want minimum DRAM space for demo for now. tag + index + dirty = 7 bits. data = 32 so 39 bit lines total
int* cache = new int[1168]; // 1 dirty + 1 valid + 2 tag + 4 index + 1 offset + 64 (32 x 2 words per line) data = 73 bits per line x 16 lines for demo = 1168 bit cache
int cycles = 0;

// our choice of write policy - we have to do write-through allocate since we have cache and dram. main memory less than address space
// need logic for valid/invalid, clean/dirty cache elements
// cache 0 delay, main memory 3 delay (3 seconds, ms or somewhere between?)
// all memory timing and commands related stuff - slides

// keep a count of each W and R command - clock cycle representation (DONE)
// command line interface (DONE)
// cache at least "16 lines" (DONE)
// need initialization function to set all array elems to 0 - cache lines start invalid and clean (DONE)

int write(string addr, string data){ //respond with "wait" or "done", write to mem or cache
    cycles++;
    cout << "write command" << endl;
    return 0;
}

int read(string addr){ //respond with "wait" or "done" and return stored value
    cycles++;
    cout << "read command" << endl;
    return 0;
}

// addr is the address we want, memLvl tells us cache or dram (1 or 0, respectively)
int view(string addr, string memLvl){ //prints the tag, index, and offset along with the data they map to - if level is 1 for cache then also valid and dirty bits
    cout << "view command" << endl;
    string dirty, valid; // cache only
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    if(memLvl == "1"){ // if we pass in a cache level
        dirty = addr.substr(7, 1);
        valid = addr.substr(8, 1);
    }
    return 0;
}

int main(){
    for(int i = 0; i < 2496; i++){ //initializing DRAM and cache to 0
        ram[i] = 0;
    }
    for(int i = 0; i < 1168; i++){
        cache[i] = 0;
    }
    while(1){
        string command, param1, param2; // command for w, r, or v, 1st parameter for command, 2nd parameter for command, not present with r
        cin >> command; //read in command and test
        if(command == "w"){
            cin >> param1;
            cin >> param2;
            cout << write(param1, param2);
        }
        else if(command == "r"){
            cin >> param1;
            cout << read(param1);
        }
        else if(command == "v"){
            cin >> param1;
            cin >> param2;
            cout << view(param1, param2);
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