#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <thread> // for threading the pipe
#include <mutex> // for locking threads on memory accesses
#include <vector> // for thread list of unknown size
#include <fstream> // read text file for commands
#include <condition_variable>
#include <future>
#include "memory.h"
#include "pipe.h"
#include "mainwindow.h"

using namespace std;

mutex mtx; // for locking threads
condition_variable cv;

pipe::pipe()
{
    this->count = 0; // used to tell whether memory is handling an access
    this->stage = 0; // no idea, included in slides
    this->global_mem;
    this->global_pc = 0;
    this->global_loop = ""; // dedicated register that holds addr of first member of a loop
    this->global_cmp = ""; // dedicated register that holds result of a CMP for future instructions
    this->ready = false;
    this->processed = false;
    this->reg[16]; // registers
}

string pipe::int_to_binary(int n) {
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

void pipe::writeback(string instruction, string data, string rn, string rd, memory mem, string reg[], int pc) {
    unique_lock<mutex> lk(mtx);
    cv.wait(lk, [this]{return ready;});

    //after wait we now own the lock - execute
    cout << "WRITEBACK" << endl;
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

    processed = true;
    ready = false;
    lk.unlock();
    cv.notify_all();
}

void pipe::memory_pipe(string instruction, string data, string rn, string rd, string shifter, memory mem, string reg[], int pc) {
    unique_lock<mutex> lk(mtx);
    cv.wait(lk, [this]{return ready;});

    //after wait we now own the lock - execute
    cout << "MEMORY" << endl;
    if(instruction == "ADD" || instruction == "CMP" || instruction == "B"){ // no interaction with memory for ALU ops or branch, just call writeback
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        writeback(instruction, data, rn, rd, mem, reg, pc);
    }
    if(instruction == "LD"){ // read the value from memory we want to store in a register. CAN STALL IF CACHE MISS.
        int prev_cycles = mem.get_cycles();
        data = mem.read(rn); // get value we want from memory
        for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
            cout << "memory stalled, LD read" << endl;
            //Sleep(300); // read stall
            
        }
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        writeback(instruction, data, rn, rd, mem, reg, pc); // call writeback to store value we got in the register
    }
    if(instruction == "STR"){
        data = reg[mem.binary_int( stoll(rd) )]; // get the data we want to store in memory from the right register
        int prev_cycles = mem.get_cycles();
        mem.write(rn, data); // write the value we got from the register to memory
        for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
            cout << "memory stalled, STR write" << endl;
            //Sleep(300); // write stall
        }
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        writeback(instruction, data, rn, rd, mem, reg, pc);
    }
    if(instruction == "NO_OP"){ // don't do anything, continue to pass blanks
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        writeback(instruction, data, rn, rd, mem, reg, pc);
    }
}

void pipe::execute(string instruction, string rn, string rd, string shifter, memory mem, string reg[], int pc) {
    unique_lock<mutex> lk(mtx);
    cv.wait(lk, [this]{return ready;});

    //after wait we now own the lock - execute
    cout << "EXECUTE" << endl;
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
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        memory_pipe(instruction, result, rn, rd, shifter, mem, reg, pc);
    }

    if(instruction == "CMP"){
        if(reg[mem.binary_int( stoll(rn) )] < reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]){ // if less than, global_cmp = 00
            rd = "00";
            cout << "CMP: FIRST OPERAND WAS LESS THAN SECOND OPERAND" << endl;
        }
        else if(reg[mem.binary_int( stoll(rn) )] > reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]){ // if greater than, global_cmp = 11
            rd = "11";
            cout << "CMP: FIRST OPERAND WAS MORE THAN SECOND OPERAND" << endl;
        }
        else{ // if gequal, global_cmp = 01
            rd = "01";
            cout << "CMP: FIRST OPERAND WAS EQUAL TO SECOND OPERAND" << endl;
        }
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc); // placeholder for data, not used in memory_pipe. the "data" is rd
    }
    
    if(instruction == "LD" || instruction == "STR"){ // load and store, just call memory pipe. Never stalls.
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc); // placeholder for data as it's created in memory_pipe for these instructions
    }
    if(instruction == "B"){ // branch - check opcode for cases, check against global_cmp. If any are true, adjust pc back to global_loop. if false, pc moves forward
        // rn is target addr (global_loop), rd is condition code
        if(rd == "0101" || rd == "0110" || rd == "0010"){ // greater than/not equal case
        cout << "BRANCH GREATER THAN/NOT EQUAL CASE" << endl;
            if(global_cmp == "11"){ // if > is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(rn) );
                cout << "CMP CONDITION WAS TRUE FOR BRANCH, BRANCH BACK TO FIRST MEMBER OF LOOP" << endl;
            }
        }
        else if(rd == "0011" || rd == "0100" || rd == "0010"){ // less than/not equal case
            cout << "BRANCH LESS THAN/NOT EQUAL CASE" << endl;
            cout << "GLOBAL LOOP: " << rn << endl;
            if(global_cmp == "00"){ // if < is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(rn) );
                cout << "CMP CONDITION WAS TRUE FOR BRANCH, BRANCH BACK TO FIRST MEMBER OF LOOP" << endl;
            }
        }
        else if(rd == "0001" || rd == "0100" || rd == "0110"){ // equals case
            cout << "BRANCH EQUAL CASE" << endl;
            if(global_cmp == "01"){ // if equals is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(rn) );
                cout << "CMP CONDITION WAS TRUE FOR BRANCH, BRANCH BACK TO FIRST MEMBER OF LOOP" << endl;
            }
        }
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc);
    }
    if(instruction == "NO_OP") {
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc); //passes in blanks, no op
    }
}

//31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
void pipe::decode(string instruction, memory mem, string reg[], int pc) {
   //deal with condition codes first
    //wait for main to give the OK
    unique_lock<mutex> lk(mtx);
    cv.wait(lk, [this]{return ready;});

    //after wait we now own the lock - execute
    cout << "DECODE" << endl;
    if(instruction.substr(0, 4) == "0111"){ // LOOP, save this addr in global_loop as 8 bit string as this is the start of a loop
        global_loop = this->int_to_binary(pc - 1);
        cout << "SAVED LOOP ADDR" << endl;
    } 
    if(instruction.substr(0,4) == "1111"){ // special no-op case
        //no op
        //pass in null params
        string rn;
        string rd;
        string shift_opt;
        processed = true;
        ready = false;
        lk.unlock();
        cv.notify_all();
        execute("NO_OP", rn, rd, shift_opt, mem, reg, pc);
    }
    else{
        string op_code = instruction.substr(6,5);
        if(op_code == "00000"){ // ADD
            cout << "ADD IN DECODE" << endl;
            string rn = instruction.substr(12,4); // register with the first operand
            string rd = instruction.substr(16,4); // destination register for result
            string shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
            processed = true;
            ready = false;
            lk.unlock();
            cv.notify_all();
            execute("ADD", rn, rd, shift_opt, mem, reg, pc);
        }
        if(op_code == "01010"){ // CMP
            cout << "CMP IN DECODE" << endl;
            string rn = instruction.substr(12,4); // register with first operand
            string rd = global_cmp; // destination register, it'll always be global_cmp so we can retain it for next instruction
            string shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
            processed = true;
            ready = false;
            lk.unlock();
            cv.notify_all();
            execute("CMP", rn, rd, shift_opt, mem, reg, pc);
        }
        if(op_code == "01111"){ // LOAD
            cout << "LD IN DECODE" << endl;
            string rn = instruction.substr(12,8); // Memory address containing value to be loaded
            string rd = instruction.substr(20,4); // Specifies the destination register, which value we want to load the value into
            processed = true;
            ready = false;
            lk.unlock();
            cv.notify_all();
            execute("LD", rn, rd, "000000000000", mem, reg, pc);
        }
        if(op_code == "10001"){ // STORE
            cout << "STR IN DECODE" << endl;
            string rn = instruction.substr(12,8); // Memory address we want to store the value to
            string rd = instruction.substr(20,4); // Specifies the register that has the value we want to store in memory
            processed = true;
            ready = false;
            lk.unlock();
            cv.notify_all();
            execute("STR", rn, rd, "000000000000", mem, reg, pc);
        }
        if(op_code == "11000"){ // BRANCH
            cout << "BRANCH IN DECODE" << endl;
            string target_address = global_loop; // address we branch to (will ALWAYS BE global_loop, set pc to this) if the condition is true, otherwise just go to pc++ address
            string condition_code = instruction.substr(0, 4); // need cond code for execute to determine to branch or not
            processed = true;
            ready = false;
            lk.unlock();
            cv.notify_all();
            execute("B", target_address, condition_code, "", mem, reg, pc); // target addr acts as rn in execute, condition code is rd. need no other data besides pc
        }
    }
}

void pipe::fetch(int pc, memory mem, string reg[]) {
    //wait for main to give the OK
    unique_lock<mutex> lk(mtx);
    cv.wait(lk, [this]{return ready;});

    //after wait we now own the lock - execute
    cout << "FETCH" << endl;
    string instruction_addr = this->int_to_binary(pc);
    int prev_cycles = mem.get_cycles();
    string instruction = mem.read(instruction_addr);
    cout << "Fetch reading instruction from memory stall" << endl;
    //Sleep(1000); // read stall
    int current_cycles = mem.get_cycles();
    if((current_cycles - prev_cycles) > 1){
        for(int i = 0; i < current_cycles-prev_cycles; i++){
            cout << "NO OP MOMENT" << endl; // no op
            string no_op = "11110000000000000000000000000000"; // cond code = 1111 for no-op
            decode(no_op, mem, reg, pc);
        }
    }
    pc++;
    processed = true;
    ready = false;
    lk.unlock();
    cv.notify_all();
    decode(instruction, mem, reg, pc);
}

memory pipe::get_mem(){
    return global_mem;
}

//string pipe::*get_reg(){
//    return reg;
//}

void pipe::run_pipe(){
    cout << "test" << endl;

    //BRANCH TEST -> THIS IS OUR DEMO PROGRAM
    string cond = "0011"; // less than cond code
    string is_branch = "0";
    string i_bit = "0";
    string opcode = "11000";
    string s_bit = "0";
    string target_address = "00000000000000000000"; // always will be global_loop where we look for this, DOESN'T MATTER
    global_mem.write("00000000", "00000001111000000110000000000000"); // write first LD instruction to mem[0] (LD mem[6] to reg[0])
    global_mem.write("00000001", "00000001111000000111000100000000"); // write second LD instruction to mem[1] (LD mem[7] to reg[1])
    global_mem.write("00000010", "00000001111000001000001000000000");// write third LD instruction to mem[2] (LD mem[8] to reg[2])
    global_mem.write("00000011", "01110000000000000000000100000000"); // write ADD command to mem[3] (ADD reg[0], reg[0], reg[1])
    global_mem.write("00000100", "00000001010000000000001000000000");// write CMP command to mem[4]. Compare reg[0] to reg[2], which holds 5 in binary. (CMP reg[0], reg[2], store result in global_cmp)
    global_mem.write("00000101", "00110011000000000000000000000000"); // write B to mem[5] (BLT global_loop)
    global_mem.write("00000110", "00000000000000000000000000000000"); // write 0 to mem[6]. we're adding 1 to this value in a loop
    global_mem.write("00000111", "00000000000000000000000000000001"); // write 1 to mem[7]. This is the "1" we're adding to em[6] every loop
    global_mem.write("00001000", "00000000000000000000000000000101"); // write 5 to mem[8]. We're CMPing this and mem[6] to see if mem[6] is less than this
     // write the value we're adding to to memory -> this should be 5 in binary once done
     // write the value we're adding to the original to memory (just 1 in binary) - shouldn't change
     
    //CONDITION HERE FROM UI ABOUT WHICH CASE TO DO - NO PIPE ETC

    // NO CONCURRENT PIPE CASE - INSTRUCTIONS 1 AT A TIME
    bool multi = false;
    /*
    while(global_pc < pc_limit){
        n++;
        thread new_thread(fetch, global_pc, global_mem, reg);
        new_thread.join();
    }*/


    // MULTITHREADED CASE - YES PIPE
    if(multi){
        vector<thread> thread_list; // list of threads, one per command
        thread t1, t2, t3, t4, t5;
        bool is_finished = false;
        t1 = thread(fetch, global_pc, global_mem, reg);
        t2 = thread(fetch, global_pc, global_mem, reg);
    //  t3 = thread(fetch, global_pc, global_mem, reg);
    // t4 = thread(fetch, global_pc, global_mem, reg);
    // t5 = thread(fetch, global_pc, global_mem, reg);
        for(int i = 0; i < 10; i++){
        {
            lock_guard<mutex> lk(mtx);
            ready = true;
            cout << "main signals ready for stage func" << endl;
        }
        cv.notify_one();

        //wait for stage func to finish
        {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [this]{return processed;});
        }

        processed = false;
        cout << "back in main. Next cycle?" << endl;
        string x;
        //cin >> x;
        }

        t1.join();
        t2.join();
    }
    else{
        for(int i = 0; i < 18; i++){
            //MainWindow::update_ram_cache(global_mem);
            thread t1 = thread(fetch, global_pc, global_mem, reg);
            for(int i = 0; i < 5; i++){
                {
                    lock_guard<mutex> lk(mtx);
                    ready = true;
                    cout << "main signals ready for stage func" << endl;
                }
                cv.notify_one();

                //wait for stage func to finish
                {
                    unique_lock<mutex> lk(mtx);
                    cv.wait(lk, [this]{return processed;});
                }

                processed = false;
                cout << "reg 0: " << reg[0] << endl;
                cout << "reg 1: " << reg[1] << endl;
                cout << "back in main. Next cycle?" << endl;
                string x;
                Sleep(100);
                //cin >> x;
             }

             t1.join();
        }


        cout << "reg 0 final result: " << reg[0] << endl;

        cout << "FULL CACHE AND RAM PRINT\nCACHE:" << endl;
        for(int i = 0; i < 16; i++){
            cout<< global_mem.get_cache()[i] << endl;
        }
        cout << "MAIN RAM:" << endl;
        for(int i = 0; i < 256; i++){
            cout<< global_mem.get_ram()[i] << endl;
        }
    }

}

