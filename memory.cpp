#include <iostream>
#include <string>
#include "memory.h"
using namespace std;

memory::memory() {
	for(int i = 0; i < 256; i++){ //initializing DRAM and cache to all 0's
	  this->ram[i] = "00000000000000000000000000000000"; // 32 character long lines in main ram
	}
	for(int i = 0; i < 16; i++){
	  this->cache[i] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"; // 138 character long lines in cache
	}
	this->cycles = 0;
}

string *memory::get_ram(){
	return this->ram;
}
string *memory::get_cache(){
	return this->cache;
}
int memory::get_cycles(){
	return this->cycles;
}