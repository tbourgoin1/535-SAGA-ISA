#include <iostream>
#include <string>
#include <fstream> // read text file for commands
#include <sstream>
#include <vector>
#include "assembler.h"

using namespace std;

int x;

string assembler::int_to_binary_assembler(int n) {
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

string assembler::operand_transform(string in){
	string out;
	if(in[in.size()-1] == ',')
		in = in.substr(0, in.size()-1);
	if(in[0] == '#' || in[0] == 'r' || in[0] == 'm'){
		// convert int to binary
		// 256 ram address so int converted to 8 bits
		out = int_to_binary_assembler(stoll(in.substr(1, in.size()-1)));
	}
	else if(in[0] == 'b'){
		// already binary
		out = in.substr(1, in.size()-1);
	}
	return out;
}

// tokenize individual instructions to be pushed to instruction list
vector<string> assembler::tokenize_line(string instruction){
	vector<string> inst;
	istringstream is(instruction);
	string token;
	while(getline(is, token, ' '))
		inst.push_back(token);
	return inst;
}

// file -> list of instructions, each element is a tokenized instruction
vector<vector<string>> assembler::vectorize_file(string filename){
	vector<vector<string>> inst_list;
	ifstream file_reader; // reads assembly file
	string instruction;
	file_reader.open(filename);
	if(!file_reader){
		cerr << "Unable to open file!";
		exit(1);
    }
    while(getline(file_reader, instruction)){ // read in all instructions from file
        //cout << "reading instruction: " << instruction << endl;
        //cout << "instruction size: " << instruction.size() << endl;
        if (instruction.size() < 3) // blank line (up to 2 chars could be \n)
        	continue;
        else
        	inst_list.push_back(tokenize_line(instruction));
    }
    return inst_list;
}

vector<string> assembler::translate_instructions(vector<vector<string>> inst_list){
	vector<string> binary_inst;
    for(int i = 0; i < inst_list.size(); i++){
    	string cond = "0000";
	    string is_branch = "0";
	    string i_bit = "0";
	    string opcode;
	    string s_bit = "0";
	    string rn;
	    string rd;
	    string shifter_operand;

        string operation = inst_list[i][0];
        string scan;
        if(operation.size() > 3){
            scan = operation.substr(operation.size()-3, 3);
            if(scan == "NEQ" || scan == "LTE" || scan == "GTE"){
                operation = operation.substr(0, operation.size()-3);
                if(scan == "NEQ")
                    cond = "0010";
                else if(scan == "LTE")
                    cond = "0100";
                else if(scan == "GTE")
                    cond = "0110";
            }
        }
        else if(operation.size() > 2){
            scan = operation.substr(operation.size()-2, 2);
            if(scan == "EQ" || scan == "LT" || scan == "GT"){
                operation = operation.substr(0, operation.size()-2);
                if(scan == "EQ")
                    cond = "0001";
                else if(scan == "LT")
                    cond = "0011";
                else if(scan == "GT")
                    cond = "0101";
            }
        }

        if(operation == "ADD" || operation == "SUB" || operation == "MUL" || operation == "DIV" || operation == "MOD" || operation == "CMP" || operation == "AND" ||
            operation == "NOT" || operation == "XOR" || operation == "OR" || operation == "LS" || operation == "RS"){

            rd = operand_transform(inst_list[i][1]).substr(4,4);
            rn = operand_transform(inst_list[i][2]).substr(4,4);
            shifter_operand = operand_transform(inst_list[i][3]).substr(4, 4) + "00000000";

        	if(operation == "ADD"){
        		opcode = "00000";
        		cout << "binary ADD inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
        	}
        	else if(operation == "SUB"){
        		opcode = "00001";
        		cout << "binary SUB inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
        	}
        	else if(operation == "MUL"){
        		opcode = "00010";
        		cout << "binary MUL inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
        	}
        	else if(operation == "DIV"){
        		opcode = "00011";
        		cout << "binary DIV inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
        	}
        	else if(operation == "MOD"){
        		opcode = "00100";
        		cout << "binary MOD inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
        	}
        	else if(operation == "CMP"){
        		opcode = "01010";
        		cout << "binary CMP inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
        	}
            else if(operation == "AND"){
                opcode = "00101";
                cout << "binary AND inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }
            else if(operation == "NOT"){
                opcode = "01000";
                cout << "binary NOT inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }
            else if(operation == "XOR"){
                opcode = "01001";
                cout << "binary XOR inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }
            else if(operation == "OR"){
                opcode = "00111";
                cout << "binary OR inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }

            string b = cond + is_branch + i_bit + opcode + s_bit  + rn + rd + shifter_operand;
            binary_inst.push_back(b);
        }
        else if(operation == "LD" || operation == "STR"){
            shifter_operand = "00000000";
            rd = operand_transform(inst_list[i][1]).substr(4,4);
            rn = operand_transform(inst_list[i][2]);
            if(operation == "LD"){
        		opcode = "01111";
        		cout << "binary LOAD inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }
        	else if(operation == "STR"){
        		opcode = "10001";
        		cout << "binary STORE inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }
            string b = cond + is_branch + i_bit + opcode + s_bit  + rn + rd + shifter_operand;
            binary_inst.push_back(b);
        }
        else if(operation == "LS" || operation == "RS"){
            rd = operand_transform(inst_list[i][1]).substr(4,4);
            rn = operand_transform(inst_list[i][2]).substr(4,4);
            shifter_operand = operand_transform(inst_list[i][3]).substr(3, 5) + "0000000";
            if(operation == "LS"){
                opcode = "10111";
                cout << "binary LS inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }
            else if(operation == "RS"){
                opcode = "10000";
                cout << "binary RS inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            }
        }
        else if(operation == "MOV"){
            rn = "0000";
            rd = operand_transform(inst_list[i][1]).substr(4,4);
            shifter_operand = operand_transform(inst_list[i][2]).substr(4, 4) + "00000000";
            opcode = "01011";
            string b = cond + is_branch + i_bit + opcode + s_bit  + rn + rd + shifter_operand;
            cout << "binary MOV inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            binary_inst.push_back(b);
        }
    	else if(operation == "B"){
    		//needs adjustment
    		opcode = "11000";
            rn = "0000";
            shifter_operand = "000000000000";
            rd = operand_transform(inst_list[i][1]).substr(4,4);
            string b = cond + is_branch + i_bit + opcode + s_bit  + rn + rd + shifter_operand;
            cout << "binary BRANCH inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + rn + " "  + rd + " "  + shifter_operand << endl;
            binary_inst.push_back(b);
    	}
    }
    return binary_inst;
}

vector<string> assembler::execute_assembler() {
	string file = "instruction.txt";
    return translate_instructions(vectorize_file(file));
}