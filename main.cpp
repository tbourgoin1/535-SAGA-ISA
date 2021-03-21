#include <iostream>
#include <string>
#include "memory.h"
using namespace std;

int count = 0; // used to tell whether memory is handling an access
int stage = 0; // no idea, included in slides

string int_to_binary(int n) {
    string r;
    while(n!=0){
        r=(n%2==0 ? "0":"1")+r; 
        n/=2;
    }
    int fill = 8 - r.length();
    string f = "";
    for(int i = 0; i < fill; i++){
        f = f + "0";
    }
    f = f + r;
    return f;
}

/**
void pipeline(int pc, memory mem) {
    fetch(pc, mem);
    decode();
    execute();
    memory_pipe();
    writeback();
}
**/

void writeback(string instruction, string data, string rd, memory mem, string reg[]) {
    if(instruction == "LD"){
            reg[mem.binary_int( stoll(rd) )] = data;
    }
}

void memory_pipe(string instruction, string rn, string rd, string shifter, memory mem, string reg[]) {
    if(instruction == "LD"){
            string data = mem.read(rn);
            writeback(instruction, data, rd, mem, reg);
    }
}

void execute(string instruction, string rn, string rd, string shifter, memory mem, string reg[]) {
    if(instruction == "LD"){
            memory_pipe(instruction, rn, rd, shifter, mem, reg);
    }
}

//31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
void decode(string data, memory mem, string reg[]) {
    if(data.substr(4,1) == "1"){
        //branch
    }
    else{
        string op_code = data.substr(6,5);
        if(op_code == "01111"){
            string rn = data.substr(12,8);
            string rd = data.substr(20,4);
            execute("LD", rn, rd, "000000000000", mem, reg);
        }
    }
}

void fetch(int pc, memory mem, string reg[]) {
    string instruction_addr = int_to_binary(pc);
    int prev_cycles = mem.get_cycles();
    string data = mem.read(instruction_addr);
    int current_cycles = mem.get_cycles();
    if((current_cycles - prev_cycles) > 1){
        for(int i = 0; i < current_cycles-prev_cycles; i++){
            continue; // no op
        }
    }
    pc++;
    decode(data, mem, reg);
}

int main(){
    cout << "test" << endl;
    memory mem;
    int pc = 0;
    string reg[16];
    mem.write("00000000", "00000001111000001000000000000000"); //rn address 00001000
    mem.write("00001000", "11111111111111111111111111111111");
    fetch(pc, mem, reg);
    cout << "reg 0: " << reg[0] << endl;
    cout << "finish";
    /**
    while(1){
        string command, param1, param2; // command for w, r, or v, 1st parameter for command, 2nd parameter for command, not present with r
        cin >> command; //read in command and test
        if(command == "w"){
            cin >> param1;
            cin >> param2;
            mem.write(param1, param2);
        }
        else if(command == "r"){
            cin >> param1;
            cout << "read returned: \n" << mem.read(param1) << endl;
        }
        else if(command == "v"){
            cin >> param1;
            cin >> param2;
            cout << "view returned: \n" << mem.view(param1, param2) << endl;
        }
        else if(command == "exit"){
            cout << "exiting program..." << endl;
            break;
        }
        else if(command == "cache"){
            for(int i = 0; i < 16; i++){
                cout<< mem.get_cache()[i] << endl;
            }
        }
        else if(command == "ram"){
            for(int i = 0; i < 256; i++){
                cout<< mem.get_ram()[i] << endl;
            }
        }
        else if(command == "cycles"){
            cout<< mem.get_cycles() << endl;
        }
        else{
            cout << "please enter a valid input!" << endl;
        }
    }
    **/
    cout << int_to_binary(4) << endl;
    return 0;
}

