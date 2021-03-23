#include <iostream>
#include <string>
#include "memory.h"
#include "mainwindow.h"
#include <QApplication>
using namespace std;

int count = 0; // used to tell whether memory is handling an access
int stage = 0; // no idea, included in slides
memory global_mem;
int global_pc = 0;
string global_loop = ""; // dedicated register that holds addr of first member of a loop
string global_cmp = ""; // dedicated register that holds result of a CMP for future instructions

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

void writeback(string instruction, string data, string rn, string rd, memory mem, string reg[], int pc) {
   // string cont; // for freezing execution after every cycle (TRANSLATE TO UI)
    //cin >> cont;
    if(instruction == "ADD" || instruction == "LD"){ // these instructions do the same thing - update registers with new data found in last step
        reg[mem.binary_int( stoll(rd) )] = data;
    }
    if(instruction == "CMP"){
        global_cmp = rd; // update the global cmp result with rd, calculated in execute
    }
    if(instruction == "STR" || instruction == "B"){
        cout << "writeback does not need to write to registers for this instruction." << endl;
    }
    if(instruction == "NO_OP"){ // do nothing
    }

    global_mem = mem;
    global_pc = pc;
}

void memory_pipe(string instruction, string data, string rn, string rd, string shifter, memory mem, string reg[], int pc) {
    //string cont; // for freezing execution after every cycle (TRANSLATE TO UI)
   // cin >> cont;
    if(instruction == "ADD" || instruction == "CMP" || instruction == "B"){ // no interaction with memory for ALU ops or branch, just call writeback
        writeback(instruction, data, rn, rd, mem, reg, pc);
    }
    if(instruction == "LD"){ // read the value from memory we want to store in a register. CAN STALL IF CACHE MISS.
        int prev_cycles = mem.get_cycles();
        data = mem.read(rn); // get value we want from memory
        for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
            cout << "memory stalled, LD read" << endl;
        }
        writeback(instruction, data, rn, rd, mem, reg, pc); // call writeback to store value we got in the register
    }
    if(instruction == "STR"){
        data = reg[mem.binary_int( stoll(rd) )]; // get the data we want to store in memory from the right register
        int prev_cycles = mem.get_cycles();
        mem.write(rn, data); // write the value we got from the register to memory
        for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
            cout << "memory stalled, STR write" << endl;
        }
        writeback(instruction, data, rn, rd, mem, reg, pc);
    }
    if(instruction == "NO_OP"){ // don't do anything, continue to pass blanks
        writeback(instruction, data, rn, rd, mem, reg, pc);
    }
}

void execute(string instruction, string rn, string rd, string shifter, memory mem, string reg[], int pc) {
    //string cont; // for freezing execution after every cycle (TRANSLATE TO UI)
    //cin >> cont;
    if(instruction == "ADD"){
        string op1 = reg[mem.binary_int( stoll(rn) )]; // gets first operand by converting rn to an index for reg[]
        string op2 = reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]; // gets second operand by converting the first 4 bits of shifter to an index for reg[]
        string result = "";
        int digit_sum = 0;
        int op1_size = op1.size() - 1;
        int op2_size = op2.size() - 1;
        while( op1_size >= 0 || op2_size >= 0 || digit_sum == 1){
            digit_sum += ((op1_size) ? op1[op1_size] - '0' : 0);
            digit_sum += ((op2_size) ? op2[op2_size] - '0' : 0);
            result = char(digit_sum % 2 + '0') + result; 
            digit_sum /= 2; // carry
            op1_size--; // move to next op1 char
            op2_size--; // move to next op2 char
        }
        memory_pipe(instruction, result, rn, rd, shifter, mem, reg, pc);
    }

    if(instruction == "CMP"){
        if(reg[mem.binary_int( stoll(rn) )] < reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]){ // if less than, global_cmp = 00
            rd = "00";
        }
        else if(reg[mem.binary_int( stoll(rn) )] > reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]){ // if greater than, global_cmp = 11
            rd = "11";
        }
        else{ // if gequal, global_cmp = 01
            rd = "01";
        }
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc); // placeholder for data, not used in memory_pipe. the "data" is rd
    }
    
    if(instruction == "LD" || instruction == "STR"){ // load and store, just call memory pipe. Never stalls.
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc); // placeholder for data as it's created in memory_pipe for these instructions
    }
    if(instruction == "B"){ // branch - check opcode for cases, check against global_cmp. If any are true, adjust pc back to global_loop. if false, pc moves forward
        // rd is condition code, rn is target addr (global_loop)
        if(rd == "0101" || rd == "0110" || rd == "0010"){ // greater than/not equal case
            if(rn == "11"){ // if > is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(global_loop) );
            }
        }
        else if(rd == "0011" || rd == "0100" || rd == "0010"){ // less than/not equal case
            if(rn == "00"){ // if < is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(global_loop) );
            }
        }
        else if(rd == "0001" || rd == "0100" || rd == "0110"){ // equals case
            if(rn == "01"){ // if equals is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(global_loop) );
            }
        }
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc);
    }
    if(instruction == "NO_OP") {
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc); //passes in blanks, no op
    }
}

//31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
void decode(string instruction, memory mem, string reg[], int pc) {
    //string cont; // for freezing execution after every cycle (TRANSLATE TO UI)
   // cin >> cont;
   //deal with condition codes first
    if(instruction.substr(0, 4) == "0111"){ // LOOP, save this addr in global_loop as 8 bit string as this is the start of a loop
        global_loop = int_to_binary(pc--);
        cout << "SAVED LOOP ADDR" << endl;
    } 
    if(instruction.substr(0,4) == "1111"){ // special no-op case
        //no op
        //pass in null params
        string rn;
        string rd;
        string shift_opt;
        execute("NO_OP", rn, rd, shift_opt, mem, reg, pc);
    }
    else{
        string op_code = instruction.substr(6,5);
        if(op_code == "00000"){ // ADD
            string rn = instruction.substr(12,4); // register with the first operand
            string rd = instruction.substr(16,4); // destination register for result
            string shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
            execute("ADD", rn, rd, shift_opt, mem, reg, pc);
        }
        if(op_code == "01010"){ // CMP
            string rn = instruction.substr(12,4); // register with first operand
            string rd = global_cmp; // destination register, it'll always be global_cmp so we can retain it for next instruction
            string shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
            execute("CMP", rn, rd, shift_opt, mem, reg, pc);
        }
        if(op_code == "01111"){ // LOAD
            string rn = instruction.substr(12,8); // Memory address containing value to be loaded
            string rd = instruction.substr(20,4); // Specifies the destination register, which value we want to load the value into
            execute("LD", rn, rd, "000000000000", mem, reg, pc);
        }
        if(op_code == "10001"){ // STORE
            string rn = instruction.substr(12,8); // Memory address we want to store the value to
            string rd = instruction.substr(20,4); // Specifies the register that has the value we want to store in memory
            execute("STR", rn, rd, "000000000000", mem, reg, pc);
        }
        if(op_code == "11000"){ // BRANCH
            string target_address = global_loop; // address we branch to (will ALWAYS BE global_loop, set pc to this) if the condition is true, otherwise just go to pc++ address
            string condition_code = instruction.substr(0, 4); // need cond code for execute to determine to branch or not
            execute("B", target_address, condition_code, "", mem, reg, pc); // target addr acts as rn in execute, condition code is rd. need no other data besides pc
        }
    }
}

void fetch(int pc, memory mem, string reg[]) {
    /*string cont; // for freezing execution after every cycle (TRANSLATE TO UI)
    cin >> cont;*/
    string instruction_addr = int_to_binary(pc);
    int prev_cycles = mem.get_cycles();
    string instruction = mem.read(instruction_addr);
    int current_cycles = mem.get_cycles();
    if((current_cycles - prev_cycles) > 1){
        for(int i = 0; i < current_cycles-prev_cycles; i++){
            cout << "NO OP MOMENT" << endl; // no op
            string no_op = "11110000000000000000000000000000"; // cond code = 1111 for no-op
            decode(no_op, mem, reg, pc);
        }
    }
    pc++;
    decode(instruction, mem, reg, pc);
}

int main(int argc, char *argv[]){
    cout << "test" << endl;
    string reg[16]; // registers
    // LD TEST
    cout << "CYCLES: " << global_mem.get_cycles() << endl;
    global_mem.write("00000000", "01110001111000001000000000000000"); //write 2nd arg to addr in 1st arg. rn address is 00001000 (addr of value we want loaded into register)
    global_mem.write("00001000", "11111111111111111000111111111111"); // write value we want loaded (2nd arg) to mem addr = rn
    fetch(global_pc, global_mem, reg);
    cout << "FINISHED LD EXECUTION. PRINTING RESULTS..." << endl;
    cout << "reg 0: " << reg[0] << endl;
    cout << "cache address 00000000: \n" << global_mem.view("00000000", "1") << endl;
    cout << "NEW PC: " << global_pc << endl;
    cout << "CYCLES: " << global_mem.get_cycles() << endl;
    cout << "finish\n\n\n\n" << endl;

    // STR TEST
    cout << "CYCLES: " << global_mem.get_cycles() << endl;
    global_mem.write("00000001", "00000010001000000100000100000000"); //write 2nd arg to addr in 1st arg. rn address is 00000100 (addr of memory we want register value in)
    reg[1] = "11111111111111111111111111111111"; // value we're going to put into the memory address above (00000001)
    fetch(global_pc, global_mem, reg);
    cout << "FINISHED STR EXECUTION. PRINTING RESULTS..." << endl;
    cout << "reg 1, should be all 1's (doesn't change): " << reg[1] << endl;
    cout << "cache address 00000100, should be all 1's (we stored reg 0's value here): \n" << global_mem.view("00000100", "1") << endl;
    cout << "NEW PC: " << global_pc << endl;
    cout << "CYCLES: " << global_mem.get_cycles() << endl;
    cout << "finish\n\n\n\n" << endl;

    // ADD TEST
    reg[2] = "00000000000000000000000000000001"; // 1st operand
    reg[3] = "00000000000000000000000000000001"; // 2nd operand
    string cond = "0000";
    string is_branch = "0";
    string i_bit = "0";
    string opcode = "00000";
    string s_bit = "0";
    string rn = "0010"; // first operand for add, reg[2] above
    string rd = "0100"; // dest register for add, we want reg[4]
    string shifter_operand = "001100000000"; // second operand register (reg[3]) + options for shift and constants
    global_mem.write("00000010", cond + is_branch + i_bit + opcode + s_bit + rn + rd + shifter_operand); // write command to memory
    fetch(global_pc, global_mem, reg);
    cout << "FINISHED ADD EXECUTION. PRINTING RESULTS..." << endl;
    cout << "REG[4], SHOULD BE 00000000000000000000000000000010: \n" + reg[4] << endl;
    cout << "NEW PC: " << global_pc << endl;
    cout << "CYCLES: " << global_mem.get_cycles() << endl;
    cout << "finish\n\n\n\n" << endl;

    // CMP TEST
    reg[2] = "00000000000000000000000000000001"; // 1st operand
    reg[3] = "00000000000000000000000000000001"; // 2nd operand
    cond = "0000";
    is_branch = "0";
    i_bit = "0";
    opcode = "01010";
    s_bit = "0";
    rn = "0010"; // first operand for add, reg[2] above
    rd = "0000"; // dest register for add, it'll ALWAYS BE GLOBAL_CMP and is set within decode(), so THIS DOESN'T MATTER
    shifter_operand = "001100000000"; // second operand register (reg[3]) + options for shift and constants
    global_mem.write("00000011", cond + is_branch + i_bit + opcode + s_bit + rn + rd + shifter_operand); // write command to memory
    fetch(global_pc, global_mem, reg);
    cout << "FINISHED CMP EXECUTION. PRINTING RESULTS..." << endl;
    cout << "RESULT (global_cmp), SHOULD BE 01: \n" + global_cmp << endl;
    cout << "NEW PC: " << global_pc << endl;
    cout << "CYCLES: " << global_mem.get_cycles() << endl;
    cout << "finish\n\n\n\n" << endl;


    //BRANCH TEST -> basically gonna be demo


cout << "FULL CACHE AND RAM PRINT\nCACHE:" << endl;
    for(int i = 0; i < 16; i++){
        cout<< global_mem.get_cache()[i] << endl;
    }
    cout << "MAIN RAM:" << endl;
    for(int i = 0; i < 256; i++){
        cout << global_mem.get_ram()[i] << endl;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
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
    return 0;
}

