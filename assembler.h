#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class assembler{
    private:
        int x;
    public:
        vector<string> execute_assembler();
        vector<string> translate_instructions(vector<vector<string>> inst_list);
        vector<vector<string>> vectorize_file(string filename);
        vector<string> tokenize_line(string instruction);
        string operand_transform(string in);
        string int_to_binary_assembler(int n);
};

#endif