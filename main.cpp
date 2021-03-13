#include <iostream>
#include <string>
using namespace std;

int ram[128000000];
int cache[100];
int cycles = 0;

// cache at least "16 lines", main memory less than address space
// need logic for valid/invalid, clean/dirty cache elements
// need initialization function to set all array elems to 0 - cache lines start invalid and clean
// cache 0 delay, main memory 3 delay (3 seconds, ms or somewhere between?)
// keep a count of each W and R command - clock cycle representation (DONE)
// command line interface (DONE)
// all memory timing and commands related stuff - slides

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