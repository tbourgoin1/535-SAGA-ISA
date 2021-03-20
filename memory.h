#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <cmath>
using namespace std;

class memory{
	private:
		string ram[256]; // 4:1 ram:cache direct mapping with offset
		string cache[16]; // bits | tag: 2, [0-1] | index: 4, [2-5] | offset: 2, [6-7] | dirty: 1, [8] | valid: 1, [9] | data1: 32, [10-41] | data2: 32, [42-73] | data3: 32, [74-105] | data4: 32, [106 - 137] - 138 character long string
		int cycles; // count of clock cycles
	public:
		memory(string ram[], string cache[], int cycles);
		string get_ram();
		string get_cache();
		int get_cycles();
	};

#endif