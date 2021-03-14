#include <iostream>
#include <string>
using namespace std;

int* ram = new int[4672]; // we want a 4:1 mapping from cache to DRAM - just want minimum DRAM space for demo for now. 1168 * 4 = 4672
int* cache = new int[1168]; // 1 dirty + 1 valid + 2 tag + 4 index + 1 offset + 64 (32 x 2 words per line) data = 73 bits per line x 16 lines for demo = 1168 bit cache
int cycles = 0;

// cache at least "16 lines", our choice of write policy - we have to do write-through allocate since we have cache and dram. main memory less than address space
// need logic for valid/invalid, clean/dirty cache elements
// need initialization function to set all array elems to 0 - cache lines start invalid and clean
// cache 0 delay, main memory 3 delay (3 seconds, ms or somewhere between?)
// all memory timing and commands related stuff - slides

// keep a count of each W and R command - clock cycle representation (DONE)
// command line interface (DONE)

int write(){ //respond with "wait" or "done"
    cycles++;
    cout << "write command" << endl;
    return 0;
}

int read(){ //respond with "wait" or "done" and return stored value
    cycles++;
    cout << "read command" << endl;
    return 0;
}

int view(){ //prints a line of values for address - if level is 1 then also tag, valid, dirty bits
    cout << "view command" << endl;
    return 0;
}

int main(){
    for(int i = 0; i < 4672; i++){ //initializing DRAM and cache to 0
        ram[i] = 0;
    }
    for(int i = 0; i < 1168; i++){
        cache[i] = 0;
    }
    while(1){
        string input;
        cin >> input;
        if(input == "w"){
            write();
        }
        else if(input == "r"){
            read();
        }
        else if(input == "v"){
            view();
        }
        else if(input == "exit"){
            cout << "exiting program..." << endl;
            break;
        }
        else{
            cout << "please enter a valid input!" << endl;
        }
    }
    return 0;
}