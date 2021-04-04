#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <fstream> // read text file for commands
#include <vector>
#include <algorithm>
#include "memory.h"
using namespace std;

memory global_mem;
int global_pc = 0;
string global_cmp = ""; // dedicated register that holds result of a CMP for future instructions

struct to_return{ // used to return from each stage
    string ins_type;
    string instruction;
    string data;
    string rn; 
    string rd;
    string shifter;
    string cond_code;
};

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

bool cond_code_helper(string cond_code){ // returns false if global_cmp and cond code don't match up, true if they do
    bool result = false;
    if(cond_code == "0101" || cond_code == "0110" || cond_code == "0010"){ // greater than/not equal case
        if(global_cmp == "11"){ // if > is true, loop back to 1st member of loop
            result = true;
        }
    }
    else if(cond_code == "0011" || cond_code == "0100" || cond_code == "0010"){ // less than/not equal case
        if(global_cmp == "00"){ // if < is true, loop back to 1st member of loop
            result = true;
        }
    }
    else if(cond_code == "0001" || cond_code == "0100" || cond_code == "0110"){ // equals case
        cout << "BRANCH EQUAL CASE" << endl;
        if(global_cmp == "01"){ // if equals is true, loop back to 1st member of loop
            result = true;
        }
    }
    return result;
}

void writeback(string instruction, string data, string rn, string rd, string cond_code, memory mem, string reg[], int pc) {
    bool is_cond_code_true = true; // defaults to true if '0000', it's unconditional so should always execute
    if(cond_code != "0000"){ // may set to false if it isn't true. Don't execute in this case.
        is_cond_code_true = cond_code_helper(cond_code);
    }
    if(instruction == "ADD" || instruction == "LD"){ // these instructions do the same thing - update registers with new data found in last step
        if(is_cond_code_true){
            reg[mem.binary_int( stoll(rd) )] = data;
        }
    }
    if(instruction == "CMP"){ // no condition on CMP
        global_cmp = rd; // update the global cmp result with rd, calculated in execute
    }
    if(instruction == "STR" || instruction == "B"){
        cout << "writeback does not need to write to registers for this instruction." << endl;
    }
    if(instruction == "NO_OP"){ // do nothing
    }

}

to_return memory_pipe(string instruction, string data, string rn, string rd, string shifter, string cond_code, memory mem, string reg[], int pc) {
    bool is_cond_code_true = true; // defaults to true if '0000', it's unconditional so should always execute
    if(cond_code != "0000"){ // may set to false if it isn't true. Don't execute in this case.
        is_cond_code_true = cond_code_helper(cond_code);
    }
    if(instruction == "ADD" || instruction == "CMP" || instruction == "B"){ // no interaction with memory for ALU ops or branch, just call writeback
    }

    if(instruction == "LD"){ // read the value from memory we want to store in a register. CAN STALL IF CACHE MISS.
        if(is_cond_code_true){
            int prev_cycles = mem.get_cycles();
            data = mem.read(rn); // get value we want from memory
            for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
                cout << "memory stalled, LD read" << endl;
                Sleep(300); // read stall   
            }
        }
    }

    if(instruction == "STR"){
        if(is_cond_code_true){
            data = reg[mem.binary_int( stoll(rd) )]; // get the data we want to store in memory from the right register
            int prev_cycles = mem.get_cycles();
            mem.write(rn, data); // write the value we got from the register to memory
            global_mem = mem;
            for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
            }
            cout << "memory stalled, STR write" << endl;
            Sleep(300); // write stall
        }
    }
    if(instruction == "NO_OP"){ // don't do anything, continue to pass blanks
    }

    to_return me;
    me.ins_type = "W";
    me.instruction = instruction;
    me.rn = rn;
    me.rd = rd;
    me.shifter = shifter;
    me.data = data;
    me.cond_code = cond_code;
    return me;
    
}

to_return execute(string instruction, string rn, string rd, string shifter, string cond_code, memory mem, string reg[], int pc) {
    string data = "";
    bool is_cond_code_true = true; // defaults to true if '0000', it's unconditional so should always execute
    if(cond_code != "0000"){ // may set to false if it isn't true. Don't execute in this case.
        is_cond_code_true = cond_code_helper(cond_code);
    }
    if(instruction == "ADD"){
        if(is_cond_code_true){
            string op1 = reg[mem.binary_int( stoll(rn) )]; // gets first operand by converting rn to an index for reg[]
            string op2 = reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]; // gets second operand by converting the first 4 bits of shifter to an index for reg[]
            int digit_sum = 0;
            int op1_size = op1.size() - 1;
            int op2_size = op2.size() - 1;
            while( op1_size >= 0 || op2_size >= 0 || digit_sum == 1){
                digit_sum += ((op1_size) ? op1[op1_size] - '0' : 0);
                digit_sum += ((op2_size) ? op2[op2_size] - '0' : 0);
                data = char(digit_sum % 2 + '0') + data; 
                digit_sum /= 2; // carry
                op1_size--; // move to next op1 char
                op2_size--; // move to next op2 char
            }
        }
    }

    if(instruction == "CMP"){ // CMP can't be conditional I don't think, so no checks for cond code 
        if(reg[mem.binary_int( stoll(rn) )] < reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]){ // if less than, global_cmp = 00
            rd = "00";
            global_cmp = rd;
        }
        else if(reg[mem.binary_int( stoll(rn) )] > reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]){ // if greater than, global_cmp = 11
            rd = "11";
            global_cmp = rd;
        }
        else{ // if equal, global_cmp = 01
            rd = "01";
            global_cmp = rd;
        }
    }
    
    if(instruction == "LD" || instruction == "STR"){ // load and store, do nothing. never stalls
    }

    if(instruction == "B"){ // branch - check cond code for cases, check against global_cmp. If any are true, adjust pc back to target addr. if false, pc moves forward
        // rn is target addr, rd AND cond_code are condition code (just happened with development, is what it is)
        if(is_cond_code_true){
            pc = mem.binary_int( stoll(rn) );
            global_pc = pc;
        }
    }
    if(instruction == "NO_OP") {
        //memory_pipe(instruction, "", rn, rd, shifter, mem, reg, pc); //passes in blanks, no op
    }
    to_return me;
    me.ins_type = "M";
    me.instruction = instruction;
    me.rn = rn;
    me.rd = rd;
    me.shifter = shifter;
    me.data = data;
    me.cond_code = cond_code;
    return me;
}

//31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
to_return decode(string instruction, memory mem, string reg[], int pc) {
    string rn;
    string rd;
    string shift_opt;
    string condition_code;
    if(instruction.substr(0,4) == "1111"){ // special no-op case
        //no op
        //pass in null param
       // execute("NO_OP", rn, rd, shift_opt, mem, reg, pc);
    }
    else{
        condition_code = instruction.substr(0, 4); // same place for all instructions, always first 4 bits for cond code
        string op_code = instruction.substr(6,5);
        if(op_code == "00000"){ // ADD
            cout << "ADD IN DECODE" << endl;
            rn = instruction.substr(12,4); // register with the first operand
            rd = instruction.substr(16,4); // destination register for result
            shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
            instruction = "ADD";
        }
        if(op_code == "01010"){ // CMP
            cout << "CMP IN DECODE" << endl;
            rn = instruction.substr(12,4); // register with first operand
            rd = global_cmp; // destination register, it'll always be global_cmp so we can retain it for next instruction
            shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
            instruction = "CMP";
        }
        if(op_code == "01111"){ // LOAD
            cout << "LD IN DECODE" << endl;
            rn = instruction.substr(12,8); // Memory address containing value to be loaded
            rd = instruction.substr(20,4); // Specifies the destination register, which value we want to load the value into
            instruction = "LD";
        }
        if(op_code == "10001"){ // STORE
            cout << "STR IN DECODE" << endl;
            rn = instruction.substr(12,8); // Memory address we want to store the value to
            rd = instruction.substr(20,4); // Specifies the register that has the value we want to store in memory
            instruction = "STR";
        }
        if(op_code == "11000"){ // BRANCH
            cout << "BRANCH IN DECODE" << endl;
            rn = instruction.substr(12, 20); // target_addr, the address we want to branch to if conditions are true
            rd = instruction.substr(0, 4); // need cond code for execute to determine to branch or not
            instruction = "B";
        }
    }

    to_return me;
    me.ins_type = "E";
    me.instruction = instruction;
    me.rn = rn;
    me.rd = rd;
    me.shifter = shift_opt;
    me.data = "";
    me.cond_code = condition_code;
    return me;
}

to_return fetch(int pc, memory mem, string reg[]) {
    string instruction_addr = int_to_binary(pc);
    int prev_cycles = mem.get_cycles();
    string instruction = mem.read(instruction_addr);
    cout << "FETCH READ STALL" << endl;
    Sleep(300); // read stall
    int current_cycles = mem.get_cycles();
    if((current_cycles - prev_cycles) > 1){
        for(int i = 0; i < current_cycles-prev_cycles; i++){
            cout << "NO OP" << endl; // no op
            string no_op = "11110000000000000000000000000000"; // cond code = 1111 for no-op
        }
    }
    pc++;
    global_pc = pc;
    to_return me;
    me.ins_type = "D";
    me.instruction = instruction;
    me.data = "";
    me.rn = ""; 
    me.rd = "";
    me.shifter = "";
    me.cond_code = "";
    return me;
}

void single_instruction_pipe_with_cache(vector<vector<string>> instructs, string reg[], int pc_limit){
    to_return ret_val;
    vector<string> new_ins;
    while(!instructs.empty()){ // until we run out of instructions
        int cur_pipe_size = 1;
        cout << "REGISTER 0: " << reg[0] << endl;
        cout << "REGISTER 1: " << reg[1] << endl;
        cout << "REGISTER 2: " << reg[2] << endl;
        cout << "CURRENT PC: " << global_pc << endl;
        for(int i = 0; i < cur_pipe_size; i++){ // look at each of the instructions in the pipe
        cout << "pipe size: " << instructs.size() << endl;
        cout << instructs[0][0] << instructs[0][1] << instructs[0][2] << instructs[0][3] << instructs[0][4] << instructs[0][5] << endl;
            if(instructs[0][0] == "F"){ // FETCH CASE
                ret_val = fetch(global_pc, global_mem, reg); //  execute fetch
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "D"){ // DECODE CASE
                ret_val = decode(instructs[0][1], global_mem, reg, global_pc); //  execute decode w/ instruction as first arg
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "E"){ // EXECUTE CASE
                ret_val = execute(instructs[0][1], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], global_mem, reg, global_pc); //  execute fetch
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "M"){ // MEMORY CASE
                ret_val = memory_pipe(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], global_mem, reg, global_pc); //  execute memory_pipe
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "W"){ // WRITEBACK CASE
                writeback(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][6], global_mem, reg, global_pc); //  execute writeback, no need for return val
                instructs.erase(instructs.begin()); // take out the instruction just used
                if(pc_limit - global_pc > 0){
                    new_ins = {"F", "", "", "", "", "", ""}; // now that the current instruction is done, add the next in
                    instructs.push_back(new_ins);
                }
            }
        }
        string x;
        cin >> x;
    }
}

void concurrent_pipe_with_cache(vector<vector<string>> instructs, bool hazard_mode, string reg[], int pc_limit){
    to_return ret_val;
    vector<string> new_ins;
    while(!instructs.empty()){ // until we run out of instructions
        int cur_pipe_size = instructs.size();
        cout << "REGISTER 0: " << reg[0] << endl;
        cout << "REGISTER 1: " << reg[1] << endl;
        cout << "REGISTER 2: " << reg[2] << endl;
        cout << "CURRENT PC: " << global_pc << endl;
        for(int i = 0; i < cur_pipe_size; i++){ // look at each of the instructions in the pipe
        cout << "pipe size: " << instructs.size() << endl;
        cout << instructs[0][0] << instructs[0][1] << instructs[0][2] << instructs[0][3] << instructs[0][4] << instructs[0][5] << instructs[0][6] << endl;
            if(instructs[0][0] == "F"){ // FETCH CASE
                ret_val = fetch(global_pc, global_mem, reg); //  execute fetch
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "D"){ // DECODE CASE
                ret_val = decode(instructs[0][1], global_mem, reg, global_pc); //  execute decode w/ instruction as first arg
                instructs.erase(instructs.begin()); // take out the instruction just used
                for(int i = 0; i < instructs.size(); i++){ // check for hazards
                    if(instructs[i][0] != "F" && instructs[i][0] != "D"){ // only check instructions ahead of current in pipe
                        bool check_rn, check_rd, check_shifter = true; // set to false depending on specific instructions' needs
                        bool need_to_squash = false; // set to true if we need to squash future instructions in pipeline
                        if(instructs[i][1] == "LD" || instructs[i][1] == "STR") { check_shifter = false; } // these instructions don't use shifter so we shouldn't check it for data hazard
                        if(instructs[i][1] == "CMP") { check_rd = false; } // these instructions don't use rd so we shouldn't check it for data hazard
                        if(instructs[i][1] == "B") { check_rn, check_rd, check_shifter = false; } // these instructions don't need data hazard checks
                        // compare rn, rd, and shifter to see if we're going to use the same ones in the future that the current ins that just decoded uses - HAZARD IF SO
                        if(check_rn || check_rd || check_shifter){
                            if(ret_val.rn == instructs[i][3]) {need_to_squash = true;}
                            if(ret_val.rd == instructs[i][4]) {need_to_squash = true;}
                            if(ret_val.shifter.substr(0, 4) == instructs[i][5].substr(0, 4)) {need_to_squash = true;}
                        }
                        cout << "NEED TO SQUASH: " << need_to_squash << endl;
                        if(need_to_squash){
                            cout << "DATA HAZARD OCCURRED, FINISHING BLOCKING INSTRUCTIONS AHEAD OF CURRENT AND HALTING CURRENT" << endl;
                            vector<vector<string>> hazard_instructs;
                            for(int j = i; j < instructs.size(); j++){ // add stalling instruction and all after it to new vector
                                if(instructs[j][0] != "F" && instructs[j][0] != "D"){ // only check instructions ahead of current in pipe
                                    hazard_instructs.push_back(instructs[j]);
                                }
                            }
                            for(int j = i; j < instructs.size(); j++){ // remove instructions added to new vector from original
                                if(instructs[j][0] != "F" && instructs[j][0] != "D"){ // only check instructions ahead of current in pipe
                                    instructs.erase(instructs.begin()+j);
                                    j = j - 1; // adjust since we deleted a member of the vector
                                }
                            }
                            concurrent_pipe_with_cache(hazard_instructs, true, reg, pc_limit); // execute those blocking instructions only to completion
                            break; // now that we've found a blocking ins and executed it, stop the loop. Continue with the instructions we have in the original vector  
                        }
                    }
                }
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list.
            }
            else if(instructs[0][0] == "E"){ // EXECUTE CASE
                int old_pc;
                bool was_branch = false;
                if(instructs[0][1] == "B"){ // if branch save old PC for later to know for control hazard
                    old_pc = global_pc;
                    was_branch = true;
                }
                ret_val = execute(instructs[0][1], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], global_mem, reg, global_pc); //  execute execute
                instructs.erase(instructs.begin()); // take out the instruction just used
                if(was_branch){ // it was a branch, check for control hazards
                    if(old_pc != global_pc){ // check old pc with potentially updated PC. if changed, CONTROL HAZARD!
                        cout << "CONTROL HAZARD DETECTED! SQUASHING INSTRUCTIONS BEHIND CURRENT BRANCH." << endl;
                        for(int i = 0; i < instructs.size(); i++){ // iterate through and delete instructions that are coming after branch
                            if(instructs[i][0] == "F" || instructs[i][0] == "D"){ // only check instructions behind current in pipe (pre-execute)
                                instructs.erase(instructs.begin()+i); // get rid of those instructions
                                i = i - 1; // adjust since we deleted a member of the instructs vector
                            }
                        }
                    }
                }
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. if we had to squash from branch, this will be right after branch
            }
            else if(instructs[0][0] == "M"){ // MEMORY CASE
                ret_val = memory_pipe(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], global_mem, reg, global_pc); //  execute memory_pipe
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "W"){ // WRITEBACK CASE
                writeback(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][6], global_mem, reg, global_pc); //  execute writeback, no need for return val
                instructs.erase(instructs.begin()); // take out the instruction just used
            }
        }
        if(instructs.size() < 5 && pc_limit - global_pc > 0 && !hazard_mode){ // pipe isn't full, add there IS a next instruction to add. so add it
            new_ins = {"F", "", "", "", "", "", ""}; 
            instructs.push_back(new_ins);
        }
        string x;
        //cin >> x;
    }
}

int main(int argc, char *argv[]){
    string reg[16]; // registers

    ifstream file_reader; // reads commands.txt for each command
     string command; // each command from file_reader
     file_reader.open("commands.txt");
     if(!file_reader){
        cerr << "Unable to open file!";
        exit(1);
    }

    int pc_limit = 0;
    while(getline(file_reader, command)){ // read in all commands from file
        cout << "current command: " << command << endl;
        if(command.substr(0, 1) == "w"){ //  write something to memory
            string param1 = command.substr(2, 8); // addr for write
            string param2 = command.substr(11, 32); // data for write
            global_mem.write(param1, param2);
        }
        else if(command.substr(0, 1) == "r"){ // read from memory
            string param1 = command.substr(2, 8); // addr for read
            cout << "read returned: \n" << global_mem.read(param1) << endl;
        }
        else if(command.substr(0, 1) == "v"){ // view memory
            string param1 = command.substr(2, 8); // addr for view
            string param2 = command.substr(11, 1); // level for view
            cout << "view returned: \n" << global_mem.view(param1, param2) << endl;
        }
        if(command.substr(44, 1) == "p"){ // additional input from the file telling us it's a pipeline instruction - so we know when to stop creating threads
            pc_limit++;
        }
     }

    //CONCURRENCY CASE
    vector<vector<string>> instructs; // string ins_type, string instruction, string data, string rn, string rd, string shifter. mem, reg[], and pc are added manually when ins actually called
    vector<string> new_ins = {"F", "", "", "", "", "", ""}; // used throughout to add new instructions
    instructs.push_back(new_ins); // first fetch instruction params now in instructs vector
    cout << "Please enter which mode you would like to execute the pipeline in:\n00 = no cache, no pipe\n01 = no cache, yes pipe\n10 = yes cache, no pipe\n11 = yes cache, yes pipe" << endl;
    string run_mode;
    while(1){
        cin >> run_mode;
        if(run_mode == "00"){
            cout << "Mode not supported yet, exiting..." << endl;
            break;
        }
        if(run_mode == "01"){
            cout << "Mode not supported yet, exiting..." << endl;
            break;
        }
        if(run_mode == "10"){
            single_instruction_pipe_with_cache(instructs, reg, pc_limit); // execute single threaded pipeline with cache
            break;
        }
        if(run_mode == "11"){
            concurrent_pipe_with_cache(instructs, false, reg, pc_limit); // execute multithreaded pipeline with cache
            break;
        }
        else{
            cout << "Incorrect input, try again!" << endl;
        }
    }
    
    cout << "Printing memory location STR wrote to..." << endl;
    cout << global_mem.view("00001010", "1") << endl;
    
    /*cout << "FULL CACHE:" << endl;
    for(int i = 0; i < 16; i++){
        cout << global_mem.get_cache()[i] << endl;
    }
    cout << "FULL RAM:" << endl;
    for(int i = 0; i < 256; i++){
        cout << global_mem.get_ram()[i] << endl;
    }*/
    return 0;
}

