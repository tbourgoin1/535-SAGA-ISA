#include <iostream>
#include <string>
#include <fstream> // read text file for commands
#include <sstream>
#include <vector>

using namespace std;

/**
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
**/
string operand_transform(string in){
	/**
	string out;
	if(in[0] == '#'){
		// convert int to binary
	}
	else if(in[0] == 'b'){
		// already binary
		if(in[in.size()-1] == ',')
			out = in.substr(1, in.size()-2);
		else
			out = in.substr(1, in.size()-1);
	}
	else if(in[0] == 'r'){
		// handle register
	}
	**/
	return in;
}

// tokenize individual instructions to be pushed to instruction list
vector<string> tokenize_line(string instruction){
	vector<string> inst;
	istringstream is(instruction);
	string token;
	while(getline(is, token, ' '))
		inst.push_back(token);
	return inst;
}

// file -> list of instructions, each element is a tokenized instruction
vector<vector<string>> vectorize_file(string filename){
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

vector<string> translate_instructions(vector<vector<string>> inst_list){
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
    	if(inst_list[i][0] == "ADD"){
    		opcode = "00000";
    		rd = inst_list[i][1];
    		rn = inst_list[i][2];
    		shifter_operand = inst_list[i][3];
    		string b = cond + is_branch + i_bit + opcode + s_bit  + operand_transform(rn) + operand_transform(rd) + operand_transform(shifter_operand);
    		cout << "binary ADD inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + operand_transform(rn) + " "  + operand_transform(rd) + " "  + operand_transform(shifter_operand) << endl;
    		binary_inst.push_back(b);
    	}
    	else if(inst_list[i][0] == "SUB"){
    		opcode = "00001";
    		rd = inst_list[i][1];
    		rn = inst_list[i][2];
    		shifter_operand = inst_list[i][3];
    		string b = cond + is_branch + i_bit + opcode + s_bit  + operand_transform(rn) + operand_transform(rd) + operand_transform(shifter_operand);
    		cout << "binary SUB inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + operand_transform(rn) + " "  + operand_transform(rd) + " "  + operand_transform(shifter_operand) << endl;
    		binary_inst.push_back(b);
    	}
    	else if(inst_list[i][0] == "MUL"){
    		opcode = "00010";
    		rd = inst_list[i][1];
    		rn = inst_list[i][2];
    		shifter_operand = inst_list[i][3];
    		string b = cond + is_branch + i_bit + opcode + s_bit  + operand_transform(rn) + operand_transform(rd) + operand_transform(shifter_operand);
    		cout << "binary MUL inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + operand_transform(rn) + " "  + operand_transform(rd) + " "  + operand_transform(shifter_operand) << endl;
    		binary_inst.push_back(b);
    	}
    	else if(inst_list[i][0] == "DIV"){
    		opcode = "00011";
    		rd = inst_list[i][1];
    		rn = inst_list[i][2];
    		shifter_operand = inst_list[i][3];
    		string b = cond + is_branch + i_bit + opcode + s_bit  + operand_transform(rn) + operand_transform(rd) + operand_transform(shifter_operand);
    		cout << "binary DIV inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + operand_transform(rn) + " "  + operand_transform(rd) + " "  + operand_transform(shifter_operand) << endl;
    		binary_inst.push_back(b);
    	}
    	else if(inst_list[i][0] == "MOD"){
    		opcode = "00100";
    		rd = inst_list[i][1];
    		rn = inst_list[i][2];
    		shifter_operand = inst_list[i][3];
    		string b = cond + is_branch + i_bit + opcode + s_bit  + operand_transform(rn) + operand_transform(rd) + operand_transform(shifter_operand);
    		cout << "binary MOD inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + operand_transform(rn) + " "  + operand_transform(rd) + " "  + operand_transform(shifter_operand) << endl;
    		binary_inst.push_back(b);
    	}
    	else if(inst_list[i][0] == "CMP"){
    		opcode = "01010";
    		rd = inst_list[i][1];
    		rn = inst_list[i][2];
    		shifter_operand = inst_list[i][3];
    		string b = cond + is_branch + i_bit + opcode + s_bit  + operand_transform(rn) + operand_transform(rd) + operand_transform(shifter_operand);
    		cout << "binary CMP inst: " << cond + " "  + is_branch + " "  + i_bit + " "  + opcode + " "  + s_bit + " "  + operand_transform(rn) + " "  + operand_transform(rd) + " "  + operand_transform(shifter_operand) << endl;
    		binary_inst.push_back(b);
    	}
    	else if(inst_list[i][0] == "LD"){
    		//needs adjustment
    		opcode = "01111";
    		cout << "binary LOAD inst: " << endl;
    	}
    	else if(inst_list[i][0] == "STR"){
    		//needs adjustment
    		opcode = "10001";
    		cout << "binary LOAD inst: " << endl;
    	}
    	else if(inst_list[i][0] == "B"){
    		//needs adjustment
    		opcode = "11000";
    		cout << "binary LOAD inst: " << endl;
    	}
    }
    return binary_inst;
}

int main() {
	string file = "instruction.txt";
    translate_instructions(vectorize_file(file));
    return 0;
}