#include <iostream>
#include <string>
#include <fstream> // read text file for commands
#include <vector>
#include <chrono> // for time of exeuction
#include "memory.h"
#include "assembler.h"
using namespace std;

memory global_mem;
int global_pc = 0;
string global_cmp = ""; // dedicated register that holds result of a CMP for future instructions
bool use_cache;
int added_time = 0;

struct to_return{ // used to return from each stage
    string ins_type;
    string instruction;
    string data;
    string rn; 
    string rd;
    string shifter;
    string cond_code;
    string i_bit;
    string s_bit;
};

string int_to_binary(int n, bool eight_bit) {
    if(eight_bit){ // returns eight bit string
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

    else{ // returns 32 bit string
        unsigned int b = (unsigned int)n;
        string binary = "";
        unsigned int mask = 0x80000000u;
        while (mask > 0)
        {
            binary += ((b & mask) == 0) ? '0' : '1';
            mask >>= 1;
        }
        return binary;
    }
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
    // these instructions do the same thing - update registers with new data found in last step
    if(instruction == "ADD" || instruction == "SUB" || instruction == "MUL" || instruction == "AND" || instruction == "OR" || instruction == "XOR" || instruction == "NOT" || instruction == "MOV" || instruction == "LD" || instruction == "LS" || instruction == "RS"){
        if(is_cond_code_true){
            reg[mem.binary_int( stoll(rd) )] = data;
        }
    }
    if(instruction == "DIV" || instruction == "MOD"){ // need extra check for divide by 0 for these
        if(data != "0"){
            if(is_cond_code_true){
                reg[mem.binary_int( stoll(rd) )] = data;
            }
        }
    }
    if(instruction == "CMP"){ // no condition on CMP
        global_cmp = rd; // update the global cmp result with rd, calculated in execute
    }
    if(instruction == "STR" || instruction == "B"){
        cout << "writeback does not need to write to registers for this instruction." << endl;
    }

}

to_return memory_pipe(string instruction, string data, string rn, string rd, string shifter, string cond_code, string s_bit, memory mem, string reg[], int pc) {
    bool is_cond_code_true = true; // defaults to true if '0000', it's unconditional so should always execute
    if(cond_code != "0000"){ // may set to false if it isn't true. Don't execute in this case.
        is_cond_code_true = cond_code_helper(cond_code);
    }

    // math ops, CMP. MOV, shifts, and branch do nothing here

    if(instruction == "LD"){ // read the value from memory we want to store in a register. CAN STALL IF CACHE MISS.
        if(is_cond_code_true){
            if(s_bit == "1"){ // we used as register holding a memory location rather than literal "m21" or something
                string temp = reg[mem.binary_int( stoll(rn) )]; // gets memory location at the index of the register passed in
                rn = temp.substr(24, 8); // changes rn for use in memory_pipe
            }
            int prev_cycles = mem.get_cycles();
            if(use_cache){ 
                data = mem.read(rn, 1); 
                added_time += 1;
            } // get value we want from memory, cache mode
            else{
                data = mem.read(rn, 0);
                added_time += 2;
            } // get value we want from memory, no cache mode
            for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
                cout << "memory stalled, LD read" << endl;
            }
        }
    }

    if(instruction == "STR"){
        if(is_cond_code_true){
            if(s_bit == "1"){ // we used as register holding a memory location rather than literal "m21" or something
                string temp = reg[mem.binary_int( stoll(rn) )]; // gets memory location at the index of the register passed in
                rn = temp.substr(24, 8); // changes rn for use in memory_pipe
            }
            data = reg[mem.binary_int( stoll(rd) )]; // get the data we want to store in memory from the right register
            int prev_cycles = mem.get_cycles();
            if(use_cache) {
                mem.write(rn, data, 1); 
                added_time += 1;
            } // write the value we got from the register to memory with cache
            else{
                mem.write(rn, data, 0);
                added_time += 2;
            } // write the value we got from the register to memory with no cache
            global_mem = mem;
            for(int i = 0; i < mem.get_cycles() - prev_cycles; i++){
                cout << "memory stalled, STR write" << endl;
            }
        }
    }

    to_return me;
    me.ins_type = "W";
    me.instruction = instruction;
    me.rn = rn;
    me.rd = rd;
    me.shifter = shifter;
    me.data = data;
    me.cond_code = cond_code;
    me.s_bit = s_bit;
    return me;
    
}

to_return execute(string instruction, string rn, string rd, string shifter, string cond_code, string i_bit, string s_bit, memory mem, string reg[], int pc) {
    string data = "";
    bool is_cond_code_true = true; // defaults to true if '0000', it's unconditional so should always execute
    if(cond_code != "0000"){ // may set to false if it isn't true. Don't execute in this case.
        is_cond_code_true = cond_code_helper(cond_code);
    }
    if(instruction == "ADD"){ 
        if(is_cond_code_true){
            int op1;
            int op2;
            if(i_bit == "1"){ // uses literal # number for 1st op
                op1 = mem.binary_int( stoll(rn) ); // converts from binary to int rather than getting index
            }
            else{ // else uses register
                op1 = mem.binary_int(stoll(reg[mem.binary_int( stoll(rn) )])); // gets first operand by converting rn to an index for reg[] and converting result into int
            }
            if(s_bit == "1"){ // uses literal # number for 2nd op
                op2 = mem.binary_int( stoll(shifter.substr(0, 4)) );
            }
            else{ // else uses register
                op2 = mem.binary_int(stoll(reg[mem.binary_int( stoll(shifter.substr(0, 4)) )])); // gets second operand by converting the first 4 bits of shifter to an index for reg[] and converting result into int
            }
            int initial_result = op1 + op2;
            data = int_to_binary(initial_result, false);
        }
    }

    if(instruction == "SUB"){
        if(is_cond_code_true){
            int op1;
            int op2;
            if(i_bit == "1"){ // uses literal # number for 1st op
                op1 = mem.binary_int( stoll(rn) ); // converts from binary to int rather than getting index
            }
            else{ // else uses register
                op1 = mem.binary_int(stoll(reg[mem.binary_int( stoll(rn) )])); // gets first operand by converting rn to an index for reg[] and converting result into int
            }
            if(s_bit == "1"){ // uses literal # number for 2nd op
                op2 = mem.binary_int( stoll(shifter.substr(0, 4)) );
            }
            else{ // else uses register
                op2 = mem.binary_int(stoll(reg[mem.binary_int( stoll(shifter.substr(0, 4)) )])); // gets second operand by converting the first 4 bits of shifter to an index for reg[] and converting result into int
            }
            int initial_result = op1 - op2;
            if(initial_result <= 0){ // MIN OF 0
                initial_result = 0;
                cout << "SUB AT MIN VALUE OF 0!!!! STAYING AT 0" << endl;
            }
            data = int_to_binary(initial_result, false);
        }
    }

    if(instruction == "MUL"){
        if(is_cond_code_true){
            int op1;
            int op2;
            if(i_bit == "1"){ // uses literal # number for 1st op
                op1 = mem.binary_int( stoll(rn) ); // converts from binary to int rather than getting index
            }
            else{ // else uses register
                op1 = mem.binary_int(stoll(reg[mem.binary_int( stoll(rn) )])); // gets first operand by converting rn to an index for reg[] and converting result into int
            }
            if(s_bit == "1"){ // uses literal # number for 2nd op
                op2 = mem.binary_int( stoll(shifter.substr(0, 4)) );
            }
            else{ // else uses register
                op2 = mem.binary_int(stoll(reg[mem.binary_int( stoll(shifter.substr(0, 4)) )])); // gets second operand by converting the first 4 bits of shifter to an index for reg[] and converting result into int
            }
            int initial_result = op1 * op2;
            data = int_to_binary(initial_result, false);
        }
    }

    if(instruction == "DIV"){
        if(is_cond_code_true){
            int op1;
            int op2;
            if(i_bit == "1"){ // uses literal # number for 1st op
                op1 = mem.binary_int( stoll(rn) ); // converts from binary to int rather than getting index
            }
            else{ // else uses register
                op1 = mem.binary_int(stoll(reg[mem.binary_int( stoll(rn) )])); // gets first operand by converting rn to an index for reg[] and converting result into int
            }
            if(s_bit == "1"){ // uses literal # number for 2nd op
                op2 = mem.binary_int( stoll(shifter.substr(0, 4)) );
            }
            else{ // else uses register
                op2 = mem.binary_int(stoll(reg[mem.binary_int( stoll(shifter.substr(0, 4)) )])); // gets second operand by converting the first 4 bits of shifter to an index for reg[] and converting result into int
            }
            if(op2 != 0){
                int initial_result = op1 / op2;
                data = int_to_binary(initial_result, false);
            }
            else{
                cout << "DIVIDE BY 0, INVALID! DIV NOT CARRIED OUT, NO RESULT STORED" << endl;
                data = "0";
            }
        }
    }

    if(instruction == "MOD"){
        if(is_cond_code_true){
            int op1;
            int op2;
            if(i_bit == "1"){ // uses literal # number for 1st op
                op1 = mem.binary_int( stoll(rn) ); // converts from binary to int rather than getting index
            }
            else{ // else uses register
                op1 = mem.binary_int(stoll(reg[mem.binary_int( stoll(rn) )])); // gets first operand by converting rn to an index for reg[] and converting result into int
            }
            if(s_bit == "1"){ // uses literal # number for 2nd op
                op2 = mem.binary_int( stoll(shifter.substr(0, 4)) );
            }
            else{ // else uses register
                op2 = mem.binary_int(stoll(reg[mem.binary_int( stoll(shifter.substr(0, 4)) )])); // gets second operand by converting the first 4 bits of shifter to an index for reg[] and converting result into int
            }
            if(op2 != 0){
                int initial_result = op1 % op2;
                data = int_to_binary(initial_result, false);
            }
            else{
                cout << "MOD BY 0, INVALID! MOD NOT CARRIED OUT, NO RESULT STORED" << endl;
                data = "0";
            }
        }
    }

    if(instruction == "AND"){
        if(is_cond_code_true){
            string op1;
            string op2;
            if(i_bit == "1"){
                op1 = "0000000000000000000000000000" + rn;
            }
            else{
                op1 = reg[mem.binary_int( stoll(rn) )]; // gets first operand by converting rn to an index for reg[]);
            }
            if(s_bit == "1"){
                op2 = "0000000000000000000000000000" + shifter.substr(0, 4);
            }
            else{
                op2 = reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]; // gets second operand by converting the first 4 bits of shifter to an index for reg[]
            }
            for(int i = 0; i < op1.length(); i++){ // look at each character of the strings, AND them together
                if(op1.substr(i, 1) == "1" && op2.substr(i, 1) == "1"){ // case where both are 1, AND true. append a 1
                    data.append("1");
                }
                else{ // one or both were 0, append a 0 
                    data.append("0");
                }
            }
        }
    }

    if(instruction == "OR"){
        if(is_cond_code_true){
            string op1;
            string op2;
            if(i_bit == "1"){
                op1 = "0000000000000000000000000000" + rn;
            }
            else{
                op1 = reg[mem.binary_int( stoll(rn) )]; // gets first operand by converting rn to an index for reg[]);
            }
            if(s_bit == "1"){
                op2 = "0000000000000000000000000000" + shifter.substr(0, 4);
            }
            else{
                op2 = reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]; // gets second operand by converting the first 4 bits of shifter to an index for reg[]
            }
            for(int i = 0; i < op1.length(); i++){ // look at each character of the strings, OR them together
                if(op1.substr(i, 1) == "1" || op2.substr(i, 1) == "1"){ // case where one or the other are 1, OR true. append a 1
                    data.append("1");
                }
                else{ // both were 0, append a 0 
                    data.append("0");
                }
            }
        }
    }

    if(instruction == "NOT"){
        if(is_cond_code_true){
            string op;
            if(i_bit == "1"){
                op = "0000000000000000000000000000" + rn;
            }
            else{
                op = reg[mem.binary_int( stoll(rn) )]; // gets first operand by converting rn to an index for reg[]);
            }
            for(int i = 0; i < op.length(); i++){ // look at each character of the string, flip its bits
                if(op.substr(i, 1) == "1"){ // current char = 1, so append a 0
                    data.append("0");
                }
                else{ // current char was 0, so append a 1
                    data.append("1");
                }
            }
        }
    }
    
    if(instruction == "XOR"){
        if(is_cond_code_true){
            string op1;
            string op2;
            if(i_bit == "1"){
                op1 = "0000000000000000000000000000" + rn;
            }
            else{
                op1 = reg[mem.binary_int( stoll(rn) )]; // gets first operand by converting rn to an index for reg[]);
            }
            if(s_bit == "1"){
                op2 = "0000000000000000000000000000" + shifter.substr(0, 4);
            }
            else{
                op2 = reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]; // gets second operand by converting the first 4 bits of shifter to an index for reg[]
            }
            for(int i = 0; i < op1.length(); i++){ // look at each character of the strings, OR them together
                if(op1.substr(i, 1) != op2.substr(i, 1)){ // case where the chars are different, append a 1
                    data.append("1");
                }
                else{ // case where the chars are the same, append a 0
                    data.append("0");
                }
            }
        }
    }

    if(instruction == "MOV"){
        if(is_cond_code_true){
            data = reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]; // gets the value we want to move into the reg at rd and sets it to data
        }
    }

    if(instruction == "CMP"){ // CMP can't be conditional I don't think, so no checks for cond code
        string op1;
        string op2;
        if(i_bit == "1"){
            op1 = "0000000000000000000000000000" + rn;
        }
        else{
            op1 = reg[mem.binary_int( stoll(rn) )]; // gets first operand by converting rn to an index for reg[]);
        }
        if(s_bit == "1"){
            op2 = "0000000000000000000000000000" + shifter.substr(0, 4);
        }
        else{
            op2 = reg[mem.binary_int( stoll(shifter.substr(0, 4)) )]; // gets second operand by converting the first 4 bits of shifter to an index for reg[]
        }
        
        if(op1 < op2){ // if less than, global_cmp = 00
            rd = "00";
            global_cmp = rd;
        }
        else if(op1 > op2){ // if greater than, global_cmp = 11
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

    if(instruction == "LS"){
        data = reg[mem.binary_int( stoll(rn) )]; // original string
        int amt_to_shift = mem.binary_int( stoll(shifter.substr(0, 5)) ); // first 5 bits = amt to shift by
        for(int i = 0; i < amt_to_shift; i++){ // erase "amt_to_shift" number of characters from the BEGINNING of the string, then append a 0 to the END
            data.erase(0, 1);
            data.append("0");
        }
    }

    if(instruction == "RS"){
        data = reg[mem.binary_int( stoll(rn) )]; // original string
        int amt_to_shift = mem.binary_int( stoll(shifter.substr(0, 5)) ); // first 5 bits = amt to shift by
        for(int i = 0; i < amt_to_shift; i++){ // erase "amt_to_shift" number of characters from the END of the string, then append a 0 to the BEGINNING
            data.erase(31, 1);
            data.insert(0, "0");
        }
    }

    if(instruction == "B"){ // branch - check cond code for cases, check against global_cmp. If any are true, adjust pc back to target addr. if false, pc moves forward
        // rn is target addr, rd AND cond_code are condition code (just happened with development, is what it is)
        if(is_cond_code_true){
            pc = mem.binary_int( stoll(rn) );
            global_pc = pc;
        }
    }
    to_return me;
    me.ins_type = "M";
    me.instruction = instruction;
    me.rn = rn;
    me.rd = rd;
    me.shifter = shifter;
    me.data = data;
    me.cond_code = cond_code;
    me.i_bit = i_bit;
    me.s_bit = s_bit;
    return me;
}

//31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
//0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
to_return decode(string instruction, memory mem, string reg[], int pc) {
    string rn;
    string rd;
    string shift_opt;
    string condition_code;

    condition_code = instruction.substr(0, 4); // same place for all instructions, always first 4 bits for cond code
    string op_code = instruction.substr(6,5); // same place for all instructions, always 5 bits at this position
    string i_bit = instruction.substr(5, 1); // always in this position
    string s_bit = instruction.substr(11, 1); // always in this position
    if(op_code == "00000"){ // ADD
        cout << "ADD IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "ADD";
    }
    if(op_code == "00001"){ // SUB
        cout << "SUB IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "SUB";
    }
    if(op_code == "00010"){ // MUL
        cout << "MUL IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "MUL";
    }
    if(op_code == "00011"){ // DIV
        cout << "DIV IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "DIV";
    }
    if(op_code == "00100"){ // MOD
        cout << "MOD IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "MOD";
    }
    if(op_code == "00101"){ // AND
        cout << "AND IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "AND";
    }
    if(op_code == "00111"){ // OR
        cout << "OR IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "OR";
    }
    if(op_code == "01000"){ // NOT
        cout << "NOT IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the ONLY operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // last 8 bits are options for shifts/constants (idk)
        instruction = "NOT";
    }
    if(op_code == "01001"){ // XOR
        cout << "XOR IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with the first operand
        rd = instruction.substr(16,4); // destination register for result
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 bits are options for shifts/constants (idk)
        instruction = "XOR";
    }
    if(op_code == "01010"){ // CMP
        cout << "CMP IN DECODE" << endl;
        rn = instruction.substr(12,4); // register with first operand
        rd = global_cmp; // destination register, it'll always be global_cmp so we can retain it for next instruction
        shift_opt = instruction.substr(20,12); // first 4 bits are register of second operand, last 8 are options for shifts/constants (idk)
        instruction = "CMP";
    }
    if(op_code == "01011"){ // MOV
        cout << "MOV IN DECODE" << endl;
        rn = instruction.substr(12,4); // UNUSED but data taken so it isn't null
        rd = instruction.substr(16,4); // destination register for the value being copied over
        shift_opt = instruction.substr(20,12); // first 4 bits are register of the operand we want to copy FROM, last 8 bits are options for shifts/constants (idk)
        instruction = "MOV";
    }
    if(op_code == "10111"){ // LS
        cout << "LS IN DECODE" << endl;
        rn = instruction.substr(12,4); // register to be shifted
        rd = instruction.substr(16,4); // destination register for the shifted value
        shift_opt = instruction.substr(20,12); // first 5 bits are the amount we're shifting by (# of places to shift), last 7 bits are unused
        instruction = "LS";
    }
    if(op_code == "10000"){ // RS
        cout << "RS IN DECODE" << endl;
        rn = instruction.substr(12,4); // register to be shifted
        rd = instruction.substr(16,4); // destination register for the shifted value
        shift_opt = instruction.substr(20,12); // first 5 bits are the amount we're shifting by (# of places to shift), last 7 bits are unused
        instruction = "RS";
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

    to_return me;
    me.ins_type = "E";
    me.instruction = instruction;
    me.rn = rn;
    me.rd = rd;
    me.shifter = shift_opt;
    me.data = "";
    me.cond_code = condition_code;
    me.i_bit = i_bit;
    me.s_bit = s_bit;
    return me;
}

to_return fetch(int pc, memory mem, string reg[]) {
    string instruction_addr = int_to_binary(pc, true);
    int prev_cycles = mem.get_cycles();
    string instruction;
    if(use_cache){ instruction = mem.read(instruction_addr, 1); } // get instruction we want from memory, cache mode
    else{ instruction = mem.read(instruction_addr, 0); } // get instruction we want from memory, no cache mode
    cout << "FETCH READ STALL" << endl;
    int current_cycles = mem.get_cycles();
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
    me.i_bit = "";
    me.s_bit = "";
    return me;
}

void single_instruction_pipe(vector<vector<string>> instructs, string reg[], int pc_limit){
    to_return ret_val;
    vector<string> new_ins;
    while(!instructs.empty()){ // until we run out of instructions
        int cur_pipe_size = 1;
        cout << "CURRENT PC: " << global_pc << endl;
        for(int i = 0; i < cur_pipe_size; i++){ // look at each of the instructions in the pipe
        cout << "pipe size: " << instructs.size() << endl;
        cout << instructs[0][0] << instructs[0][1] << instructs[0][2] << instructs[0][3] << instructs[0][4] << instructs[0][5] << endl;
            if(instructs[0][0] == "F"){ // FETCH CASE
                ret_val = fetch(global_pc, global_mem, reg); //  execute fetch
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "D"){ // DECODE CASE
                ret_val = decode(instructs[0][1], global_mem, reg, global_pc); //  execute decode w/ instruction as first arg
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "E"){ // EXECUTE CASE
                ret_val = execute(instructs[0][1], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], instructs[0][7], instructs[0][8], global_mem, reg, global_pc); //  execute fetch
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "M"){ // MEMORY CASE
                ret_val = memory_pipe(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], instructs[0][8], global_mem, reg, global_pc); //  execute memory_pipe
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "W"){ // WRITEBACK CASE
                writeback(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][6], global_mem, reg, global_pc); //  execute writeback, no need for return val
                instructs.erase(instructs.begin()); // take out the instruction just used
                if(pc_limit - global_pc > 0){
                    new_ins = {"F", "", "", "", "", "", "", "", ""}; // now that the current instruction is done, add the next in
                    instructs.push_back(new_ins);
                }
            }
        }
        string x;
      //  cin >> x;
    }
}

void concurrent_pipe(vector<vector<string>> instructs, bool hazard_mode, string reg[], int pc_limit){
    to_return ret_val;
    vector<string> new_ins;
    while(!instructs.empty()){ // until we run out of instructions
        int cur_pipe_size = instructs.size();

        for(int i = 0; i < cur_pipe_size; i++){ // look at each of the instructions in the pipe
        cout << "pipe size: " << instructs.size() << endl;
        cout << instructs[0][0] << instructs[0][1] << instructs[0][2] << instructs[0][3] << instructs[0][4] << instructs[0][5] << instructs[0][6] << endl;
            if(instructs[0][0] == "F"){ // FETCH CASE
                ret_val = fetch(global_pc, global_mem, reg); //  execute fetch
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "D"){ // DECODE CASE
                ret_val = decode(instructs[0][1], global_mem, reg, global_pc); //  execute decode w/ instruction as first arg
                instructs.erase(instructs.begin()); // take out the instruction just used
                for(int i = 0; i < instructs.size(); i++){ // check for hazards
                    if(instructs[i][0] != "F" && instructs[i][0] != "D"){ // only check instructions ahead of current in pipe
                        bool check_rn = true, check_rd = true, check_shifter = true; // set to false depending on specific instructions' needs
                        bool need_to_squash = false; // set to true if we need to squash future instructions in pipeline
                        if(instructs[i][1] == "LD" || instructs[i][1] == "STR" || instructs[i][1] == "NOT" || instructs[i][1] == "LS" || instructs[i][1] == "RS") { check_shifter = false; } // these instructions don't use shifter so we shouldn't check it for data hazard
                        if(instructs[i][1] == "CMP") { check_rd = false; } // these instructions don't use rd so we shouldn't check it for data hazard
                        if(instructs[i][1] == "MOV") { check_rn = false; } // these instructions don't use rn so we shouldn't check it for data hazard
                        if(instructs[i][1] == "B") { check_rn, check_rd, check_shifter = false; } // these instructions don't need data hazard checks
                        //compare rn, rd, and shifter to see if we're going to use the same ones in the future that the current ins that just decoded uses - HAZARD IF SO
                        string cur_rn = ret_val.rn;
                        string cur_rd = ret_val.rd;
                        string cur_shifter = ret_val.shifter.substr(0, 4);
                        string other_rn = instructs[i][3];
                        string other_rd = instructs[i][4];
                        string other_shifter = instructs[i][5].substr(0, 4);
                        if(check_rn){
                            if(cur_rn.compare(other_rn) == 0 || cur_rn.compare(other_rd) == 0 || cur_rn.compare(other_shifter) == 0) {need_to_squash = true;}
                        }
                        if(check_rd){
                            if(cur_rd.compare(other_rn) == 0 || cur_rd.compare(other_rd) == 0 || cur_rd.compare(other_shifter) == 0) {need_to_squash = true;}
                        }
                        if(check_shifter){
                            if(cur_shifter.compare(other_rn) == 0 || cur_shifter.compare(other_rd) == 0 || cur_shifter.compare(other_shifter) == 0) {need_to_squash = true;}
                        }  
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
                            concurrent_pipe(hazard_instructs, true, reg, pc_limit); // execute those blocking instructions only to completion
                            break; // now that we've found a blocking ins and executed it, stop the loop. Continue with the instructions we have in the original vector  
                        }
                    }
                }
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list.
            }
            else if(instructs[0][0] == "E"){ // EXECUTE CASE
                int old_pc;
                bool was_branch = false;
                if(instructs[0][1] == "B"){ // if branch save old PC for later to know for control hazard
                    old_pc = global_pc;
                    was_branch = true;
                }
                ret_val = execute(instructs[0][1], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], instructs[0][7], instructs[0][8], global_mem, reg, global_pc); //  execute execute
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
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. if we had to squash from branch, this will be right after branch
            }
            else if(instructs[0][0] == "M"){ // MEMORY CASE
                ret_val = memory_pipe(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][5], instructs[0][6], instructs[0][8], global_mem, reg, global_pc); //  execute memory_pipe
                instructs.erase(instructs.begin()); // take out the instruction just used
                new_ins = {ret_val.ins_type, ret_val.instruction, ret_val.data, ret_val.rn, ret_val.rd, ret_val.shifter, ret_val.cond_code, ret_val.i_bit, ret_val.s_bit}; // create new instruction with data gotten from fetch 
                instructs.push_back(new_ins); // add the new instruction to the end of our instructions list. size remains 5
            }
            else if(instructs[0][0] == "W"){ // WRITEBACK CASE
                writeback(instructs[0][1], instructs[0][2], instructs[0][3], instructs[0][4], instructs[0][6], global_mem, reg, global_pc); //  execute writeback, no need for return val
                instructs.erase(instructs.begin()); // take out the instruction just used
            }
        }
        if(instructs.size() < 5 && pc_limit - global_pc > 0 && !hazard_mode){ // pipe isn't full, add there IS a next instruction to add. so add it
            new_ins = {"F", "", "", "", "", "", "", "", ""}; 
            instructs.push_back(new_ins);
        }
        string x;
       // cin >> x;
    }
}

int main(int argc, char *argv[]){
    string reg[16]; // registers

    ifstream file_reader; // reads commands.txt for each command
     string command; // each command from file_reader
     file_reader.open("program_setup.txt"); 
     if(!file_reader){
        cerr << "Unable to open file!";
        exit(1);
    }

    //decide whether to use cache or not
    cout << "\nUse cache? y/n" << endl;
    string cache_mode;
    while(1){
        cin >> cache_mode;
        if(cache_mode == "n"){
            use_cache = false;
            break;
        }
        if(cache_mode == "y"){
            use_cache = true;
            break;
        }
        else{
            cout << "Incorrect input, try again!" << endl;
        }
    }
    
    int pc_limit = 0;
    // any values that need to be written to memory (or other commands) before the instructions start executing (for LDs, for example)
    while(getline(file_reader, command)){ // read in all memory writes, reads, or views in binary from file
        if(command.substr(0, 1) == "w"){ //  write something to memory
            string param1 = command.substr(2, 8); // addr for write
            string param2 = command.substr(11, 32); // data for write
            if(use_cache) { global_mem.write(param1, param2, 1); } // write setup val to mem with cache
            else{ global_mem.write(param1, param2, 0); } // write setup val to mem with no cache
        }
        else if(command.substr(0, 1) == "r"){ // read from memory
            string param1 = command.substr(2, 8); // addr for read
            if(use_cache){ cout << "read returned: \n" << global_mem.read(param1, 1) << endl; } // read setup val from mem with cache
            else{ cout << "read returned: \n" << global_mem.read(param1, 0) << endl; } // read setup val from mem with no cache
        }
        else if(command.substr(0, 1) == "v"){ // view memory
            string param1 = command.substr(2, 8); // addr for view
            string param2 = command.substr(11, 1); // level for view
            cout << "view returned: \n" << global_mem.view(param1, param2) << endl;
        }
    }   

    assembler as;
    vector<string> binary_ins_list = as.execute_assembler(); // Parse instructions with assembler - receive vector of binary instructions

    //LOOP THROUGH VECTOR, WRITE EACH INSTRUCTION TO MEMORY STARTING AT 00000000. COUNT UP PC_LIMIT PER WRITE
    cout << "instruction list size: " << binary_ins_list.size() << endl;
    for(int i = 0; i < binary_ins_list.size(); i++){
        cout << "addr: " << int_to_binary(i, true) << "\ndata: " << binary_ins_list[i] << endl;
        if(use_cache) { global_mem.write(int_to_binary(i, true), binary_ins_list[i], 1); } // write ins to memory with cache
        else{ global_mem.write(int_to_binary(i, true), binary_ins_list[i], 0); } // write ins to memory without cache
        pc_limit++;
    }

    cout << "\n\n\nFULL CACHE PRINT:" << endl;
    for(int i = 0; i < 16; i++){
        cout << "cache position #" << i << ": " << global_mem.get_cache()[i] << endl;
    }
    cout << "\n\n\nFULL RAM PRINT:" << endl;
    for(int i = 0; i < 256; i++){
        cout << "ram position #" << i << ": " << global_mem.get_ram()[i] << endl;
    }

    

    vector<vector<string>> instructs; // string ins_type, string instruction, string data, string rn, string rd, string shifter. mem, reg[], and pc are added manually when ins actually called
    vector<string> new_ins = {"F", "", "", "", "", "", "", "", ""}; // used throughout to add new instructions
    instructs.push_back(new_ins); // first fetch instruction params now in instructs vector
    cout << "Use the pipeline? y/n" << endl;
    string run_mode;
    chrono::system_clock::time_point start; // start of function execution time
    // first bit is cache option, second is pipe option
    // 0 = no, 1 = yes
    while(1){
        cin >> run_mode;
        if(run_mode == "n"){
            start = chrono::system_clock::now();
            single_instruction_pipe(instructs, reg, pc_limit); // execute single threaded pipeline
            break;
        }
        if(run_mode == "y"){
            start = chrono::system_clock::now();
            concurrent_pipe(instructs, false, reg, pc_limit); // execute multithreaded pipeline
            break;
        }
        else{
            cout << "Incorrect input, try again!" << endl;
        }
    }

    chrono::system_clock::time_point end = chrono::system_clock::now(); // end of function execution time
    
    cout << "\n\n\nFULL CACHE PRINT:" << endl;
    for(int i = 0; i < 16; i++){
        cout << "cache position #" << i << ": " << global_mem.get_cache()[i] << endl;
    }
    cout << "\n\n\nFULL RAM PRINT:" << endl;
    for(int i = 0; i < 256; i++){
        cout << "ram position #" << i << ": " << global_mem.get_ram()[i] << endl;
    }

    cout << "\n\n\nRAM[25] - RAM[55] PRINT (FOR EXCHANGE SORT BENCHMARK):" << endl;
    for(int i = 25; i < 56; i++){
        cout << global_mem.get_ram()[i] << endl;
    }

    cout << "\n\n\nRAM[170] - RAM[250] PRINT (FOR MATRIX MULTIPLY BENCHMARK):" << endl;
    for(int i = 170; i < 251; i++){
        cout << global_mem.get_ram()[i] << endl;
    } 
    
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    int time = duration.count() + added_time;
    cout << "\nTime to run: " << time << "ms" << endl;
    
    return 0;
}