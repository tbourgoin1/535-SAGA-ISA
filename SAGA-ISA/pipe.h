#ifndef PIPE_H
#define PIPE_H

#include <string>
#include "memory.h"

using namespace std;

string int_to_binary(int n);
void writeback(string instruction, string data, string rn, string rd, memory mem, string reg[], int pc);
void memory_pipe(string instruction, string data, string rn, string rd, string shifter, memory mem, string reg[], int pc);
void execute(string instruction, string rn, string rd, string shifter, memory mem, string reg[], int pc);
void decode(string instruction, memory mem, string reg[], int pc);
void fetch(int pc, memory mem, string reg[]);
void update_ram_cache(memory mem);
memory get_mem();
string *get_reg();
int run_pipe();

using namespace std;

#endif
