#ifndef PIPE_H
#define PIPE_H

#include <string>
#include <future>
#include "memory.h"

using namespace std;

class pipe{
    private:
        int count = 0; // used to tell whether memory is handling an access
        int stage = 0; // no idea, included in slides
        memory global_mem;
        int global_pc = 0;
        string global_loop = ""; // dedicated register that holds addr of first member of a loop
        string global_cmp = ""; // dedicated register that holds result of a CMP for future instructions
        bool ready = false;
        bool processed = false;
        string reg[16]; // registers
    public:
        pipe();
        string int_to_binary(int n);
        void writeback(string instruction, string data, string rn, string rd, memory mem, string reg[], int pc);
        void memory_pipe(string instruction, string data, string rn, string rd, string shifter, memory mem, string reg[], int pc);
        void execute(string instruction, string rn, string rd, string shifter, memory mem, string reg[], int pc);
        void decode(string instruction, memory mem, string reg[], int pc);
        void fetch(int pc, memory mem, string reg[]);
        void update_ram_cache(memory mem);
        memory get_mem();
        string *get_reg();
        void run_pipe();
    };

#endif
