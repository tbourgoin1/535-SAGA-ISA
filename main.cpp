#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <fstream> // read text file for commands
#include <vector>
#include "memory.h"
using namespace std;

int count = 0; // used to tell whether memory is handling an access
int stage = 0; // no idea, included in slides
memory global_mem;
int global_pc = 0;
string global_loop = ""; // dedicated register that holds addr of first member of a loop
string global_cmp = ""; // dedicated register that holds result of a CMP for future instructions

struct to_return{ // used to return from each stage
    string ins_type;
    string instruction;
    string data;
    string rn; 
    string rd;
    string shifter;
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
    if(instruction == "ADD" || instruction == "LD"){ // these instructions do the same thing - update registers with new data found in last step
        reg[mem.binary_int( stoll(rd) )] = data;
    }
    if(instruction == "CMP"){
        global_cmp = rd; // update the global cmp result with rd, calculated in execute
    }
    if(instruction == "STR"){
        cout << "writeback does not need to write to registers for this instruction." << endl;
    }
    if(instruction == "B"){
        global_pc = pc;
    }
    if(instruction == "NO_OP"){ // do nothing
    }

}

to_return memory_pipe(string instruction, string data, string rn, string rd, string shifter, memory mem, string reg[], int pc) {
    if(instruction == "ADD" || instruction == "CMP" || instruction == "B"){ // no interaction with memory for ALU ops or branch, just call writeback
    }

    if(instruction == "LD"){ // read the value from memory we want to store in a register. CAN STALL IF CACHE MISS.
        int prev_cycles = mem.get_cycles();
        data = mem.read(rn); // get value we want from memory
        for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
            cout << "memory stalled, LD read" << endl;
            Sleep(300); // read stall   
        }
    }

    if(instruction == "STR"){
        data = reg[mem.binary_int( stoll(rd) )]; // get the data we want to store in memory from the right register
        int prev_cycles = mem.get_cycles();
        mem.write(rn, data); // write the value we got from the register to memory
        global_mem = mem;
        for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
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
    return me;
    
}

to_return execute(string instruction, string rn, string rd, string shifter, memory mem, string reg[], int pc) {
    string data = "";
    if(instruction == "ADD"){
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

    if(instruction == "CMP"){
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

    if(instruction == "B"){ // branch - check opcode for cases, check against global_cmp. If any are true, adjust pc back to global_loop. if false, pc moves forward
        // rn is target addr (global_loop), rd is condition code
        if(rd == "0101" || rd == "0110" || rd == "0010"){ // greater than/not equal case
            if(global_cmp == "11"){ // if > is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(rn) );
                global_pc = pc;
            }
        }
        else if(rd == "0011" || rd == "0100" || rd == "0010"){ // less than/not equal case
            if(global_cmp == "00"){ // if < is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(rn) );
                global_pc = pc;
            }
        }
        else if(rd == "0001" || rd == "0100" || rd == "0110"){ // equals case
            cout << "BRANCH EQUAL CASE" << endl;
            if(global_cmp == "01"){ // if equals is true, loop back to 1st member of loop
                pc = mem.binary_int( stoll(rn) );
                global_pc = pc;
            }
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
    return me;
}

//31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
to_return decode(string instruction, memory mem, string reg[], int pc) {
   //deal with condition codes first
    if(instruction.substr(0, 4) == "0111"){ // LOOP, save this addr in global_loop as 8 bit string as this is the start of a loop
        global_loop = int_to_binary(pc - 1);
    }
    string rn;
    string rd;
    string shift_opt;
    if(instruction.substr(0,4) == "1111"){ // special no-op case
        //no op
        //pass in null param
       // execute("NO_OP", rn, rd, shift_opt, mem, reg, pc);
    }
    else{
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
            rn = global_loop; // address we branch to (will ALWAYS BE global_loop, set pc to this) if the condition is true, otherwise just go to pc++ address
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
           // decode(no_op, mem, reg, pc);
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
    return me;
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

   /* // LD TEST
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


    // CMP AND LOOP JOINT TEST
    reg[2] = "00000000000000000000000000000001"; // 1st operand, 1
    reg[3] = "00000000000000000000000000000001"; // 2nd operand, 1
    cond = "0111"; // should save loop addr
    is_branch = "0";
    i_bit = "0";
    opcode = "01010";
    s_bit = "0";
    rn = "0010"; // first operand, reg[2] above
    rd = "0000"; // dest register, it'll ALWAYS BE GLOBAL_CMP and is set within decode(), so THIS DOESN'T MATTER
    shifter_operand = "001100000000"; // second operand register (reg[3]) + options for shift and constants
    global_mem.write("00000011", cond + is_branch + i_bit + opcode + s_bit + rn + rd + shifter_operand); // write command to memory
    fetch(global_pc, global_mem, reg);
    cout << "FINISHED CMP EXECUTION. PRINTING RESULTS..." << endl;
    cout << "RESULT (global_cmp), SHOULD BE 01: " + global_cmp << endl;
    cout << "GLOBAL LOOP, SHOULD BE BINARY VERSION OF THIS INSTRUCTION'S PC (3): " << global_loop << endl;
    cout << "NEW PC: " << global_pc << endl;
    cout << "CYCLES: " << global_mem.get_cycles() << endl;
    cout << "finish\n\n\n\n" << endl;

*/
    //BRANCH TEST -> THIS IS OUR DEMO PROGRAM
    /*string cond = "0011"; // less than cond code
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
     
     while(1){
        fetch(global_pc, global_mem, reg);
        cout << "reg 0. Should have 00000000000000000000000000000101 at the end of execution: " << reg[0] << endl;
     }*/

    //CONCURRENCY CASE
    // vector always of size 5 that'll hold our current instructions. has ins_type and all possible args for each stage
    vector<vector<string>> instructs; // string ins_type, string instruction, string data, string rn, string rd, string shifter. mem, reg[], and pc are added manually when ins actually called
    vector<string> new_ins = {"F", "", "", "", "", ""}; // used throughout to add new instructions
    instructs.push_back(new_ins); // first fetch instruction params now in instructs vector
    to_return ret_val;
    while(!instructs.empty()){ // until we run out of instructions
        int cur_pipe_size = instructs.size();
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
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "D"){ // DECODE CASE
                ret_val = decode(instructs[0][1], global_mem, reg, global_pc); //  execute decode w/ instruction as first arg
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "E"){ // EXECUTE CASE
                ret_val = execute(instructs[0][1], instructs[0][3], instructs[0][4], instructs[0][5], global_mem, reg, global_pc); //  execute fetch
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "M"){ // MEMORY CASE
                ret_val = memory_pipe(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][5], global_mem, reg, global_pc); //  execute memory_pipe
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "W"){ // WRITEBACK CASE
                writeback(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], global_mem, reg, global_pc); //  execute writeback, no need for return val
                instructs.erase(instructs.begin()); // take out the instruction just used
            }
        }
        if(instructs.size() < 5 && pc_limit - global_pc > 0){ // pipe isn't full, add there IS a next instruction to add. so add it
            new_ins = {"F", "", "", "", "", ""}; 
            instructs.push_back(new_ins);
        }

    }




   /* cout << "FULL CACHE AND RAM PRINT\nCACHE:" << endl;
    for(int i = 0; i < 16; i++){
        cout<< global_mem.get_cache()[i] << endl;
    }
    cout << "MAIN RAM:" << endl;
    for(int i = 0; i < 256; i++){
        cout<< global_mem.get_ram()[i] << endl;
    }*/

    /*QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();*/
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

