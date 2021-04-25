#include <iostream>
#include <string>
#include <cmath>
#include "memory.h"
using namespace std;

memory::memory() {
	for(int i = 0; i < 256; i++){ //initializing DRAM and cache to all 0's
	  this->ram[i] = "00000000000000000000000000000000"; // 32 character long lines in main ram
	}
	for(int i = 0; i < 16; i++){ // bits | tag: 2, [0-1] | index: 4, [2-5] | offset: 2, [6-7] | dirty: 1, [8] | valid: 1, [9] | data1: 32, [10-41] | data2: 32, [42-73] | data3: 32, [74-105] | data4: 32, [106 - 137] - 138 character long string
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

//converts binary to integer value
int memory::binary_int(long long n){
    int num = 0, i = 0, remainder;
      while (n!=0)
      {
          remainder = n%10;
          n /= 10;
          num += remainder*pow(2,i);
          ++i;
      }
      return num;
}

string memory::cache_write(string tag, string index, string offset, string dirty, string valid, string data, int cache_address){
    string new_write;
    if(offset == "00") //if 00 then write new data to 1st cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + data + this->cache[cache_address].substr(42, 32) + this->cache[cache_address].substr(74, 32) + this->cache[cache_address].substr(106, 32);
    else if(offset == "01") //if 01 then write new data to 2nd cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + this->cache[cache_address].substr(10, 32) + data + this->cache[cache_address].substr(74, 32) + this->cache[cache_address].substr(106, 32);
    else if(offset == "10") //if 10 then write new data to 3rd cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + this->cache[cache_address].substr(10, 32) + this->cache[cache_address].substr(42, 32) + data + this->cache[cache_address].substr(106, 32);
    else if(offset == "11") //if 11 then write new data to 4th cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + this->cache[cache_address].substr(10, 32) + this->cache[cache_address].substr(42, 32) + this->cache[cache_address].substr(74, 32) + data;
    return new_write;
}

int memory::write(string addr, string data, int mode){ //respond with "wait" or "done", write to mem or cache
    if(addr.size() != 8 || data.size() != 32){ // address size must be 8 and data size must be 32
        cout << "incorrect parameter format passed to write. Try again!" << endl;
        return 1;
    }
    //parse input
    string dirty = "1"; // writing to cache will always activate dirty bit
    string valid = "1";
    string tag = addr.substr(0, 2); // 2 bits for tag
    string index = addr.substr(2, 4); // 4 bits for index
    string offset = addr.substr(6, 2); // 2 bits for offset
    int address = memory::binary_int(stoll(index));
    
    cout << "writing to address " << address << endl;
    
    if(mode == 1){ // cache mode
        if(this->cache[address][9] == '0'){ // checking valid bit - invalid case. Can just write as it's empty.
            this->cache[address] = memory::cache_write(tag, index, offset, dirty, valid, data, address);
            cycles++;
        }
        else if(this->cache[address][9] == '1'){ // checking valid bit - valid case. Need to do additional checks as it's been written to before
            if(this->cache[address][8] == '1'){ // check dirty bit, if 1 then need to write back to ram before writing to cache
                // write back all 4 words in cache line to memory
                int ram_address = memory::binary_int( stoll(this->cache[address].substr(0,2) + index + offset) );
                this->ram[ram_address] = this->cache[address].substr(10, 32); // data1
                this->ram[ram_address+1] = this->cache[address].substr(42, 32); // data2
                this->ram[ram_address+2] = this->cache[address].substr(74, 32); // data3
                this->ram[ram_address+3] = this->cache[address].substr(106, 32); // data4
                this->cycles = this->cycles + 12; // (4 * 3) = 12 cycles for 4 memory accesses
            }
            this->cache[address] = memory::cache_write(tag, index, offset, dirty, valid, data, address);
            this->cycles++; // +1 for cache access
            cout << "cache write/eviction/memory write back stall" << endl;
        }
    }
    else{ // no cache mode, just write to main memory
        int ram_address = memory::binary_int( stoll(tag + index + offset) ); // index of array of ram we need to write to (address we're writing to)
        this->ram[ram_address] = data; // write
        this->cycles = this->cycles + 3; // 3 cycles for ram write
        cout << "RAM write no cache mode stall" << endl;
    }

    cout << "cycle count: " << this->cycles << endl; // after every write print the # of cycles
    return 0;
}

// addr is the address we want, memLvl tells us cache or dram (1 or 0, respectively)
string memory::view(string addr, string memLvl){ //prints the tag, index, and offset along with the data they map to - if level is 1 for cache then also valid and dirty bits
    if(addr.size() != 8 || memLvl.size() != 1){ // address size must be 8 and data size must be 32
        cout << "incorrect parameter format passed to view. Try again!" << endl;
        return "0";
    }
    //parse input
    string dirty, valid; // cache only, need to get from the addr passed in by searching cache
    string data; //data we find from viewing
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 2);
    bool found = false; // for ending if statements, lets us know if we found the address in cache or ram

    if(memLvl == "1"){ // case to read from cache. If we find matching index and tag for a line - CACHE HIT
        int cache_address = memory::binary_int(stoll(index));
        string line = this->cache[cache_address];
        if(tag == line.substr(0,2)){
            dirty = line.substr(8, 1);
            valid = line.substr(9, 1);
            data = line.substr(10, 32) + " " + line.substr(42, 32) + " " + line.substr(74, 32) + " " + line.substr(106, 32);
            found = true;
        }
    }

    else{ // case to read from memory. Should always find a data value
        int ram_address = memory::binary_int(stoll(tag + index + offset));
        found = true;
        string line = this->ram[ram_address];
        data = line;
    }

    if(memLvl == "1" && !found){ //couldn't find address given in cache
        return "Address not found in cache!";
    }
    else if(!found && memLvl == "0"){ // addr not found in RAM, something's wrong with the addr given
        return "address not found in main RAM! Wrong format?";
    }
    else if(memLvl == "1" && found){ // tag+index address combo found in cache - return tag, valid, dirty, and data pieces
        return "Cache info: \nTag: " + tag + "\nIndex: " + index + "\nOffset: " + offset + "\nValid: " + valid + "\nDirty: " + dirty + "\nData: " + data + "\n";
    }
    else{ // final case = we asked for and found addr in main RAM
        return "Main RAM data for address " + addr + ": " + data;
    }
}

string memory::read(string addr, int mode){ //respond with "wait" or "done" and return stored value
    if(addr.size() != 8){ // address size must be 8
        cout << "incorrect parameter format passed to read. Try again!" << endl;
        return 0;
    }
    //parse input
    string data; //data we find from reading, if applicable
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 2);
    int cache_address = memory::binary_int(stoll(index)); //decimal version of binary cache addr for indexing 
	string dirty = cache[cache_address].substr(8, 1);
	string valid = cache[cache_address].substr(9, 1);

    if(mode == 1){ // use the cache
        if(valid == "0") {
            int ram_address = memory::binary_int( stoll( tag + index + "00" ) );
            string new_write = tag + index + offset + dirty + "1" + this->ram[ram_address] + this->ram[ram_address+1] + this->ram[ram_address+2] + this->ram[ram_address+3];
            this->cache[cache_address] = new_write;
        }

        string line = this->cache[cache_address]; // the line of cache we want to initially look at (matches index)
        this->cycles++;
        if(tag == line.substr(0, 2)){ // found the index in the cache, now make sure the tags equal. if so, CACHE HIT
            if(offset == "00") //if offset is 00, get first piece of data (word) in line
                data = line.substr(10, 32);
            else if(offset == "01") //if offset is 01, get second piece of data (word) in line
                data = line.substr(42, 32);
            else if(offset == "10") //if offset is 10, get third piece of data (word) in line
                data = line.substr(74, 32);
            else if(offset == "11") //if offset is 11, get fourth piece of data (word) in line
                data = line.substr(106, 32);
            else //offset is warped
                return "In read - cache hit but offset is NOT CORRECT";
            return data; // cache hit, return data
        }

        else{ //else the tags didn't equal, so we have a CACHE MISS
            //find the piece of data we want to read in main memory
            int ram_address = memory::binary_int( stoll( tag + index + offset ) );
            string dirty = "0";
            string valid = "1";
            string old_tag = this->cache[cache_address].substr(0, 2);
            int old_ram_address = memory::binary_int(stoll(old_tag + index + "00"));
            for(int i = 0; i < 4; i++){ // write back to ram
                this->ram[old_ram_address + i] = this->cache[cache_address].substr(10 + (32 * i), 32);
            }
            this->cache[cache_address] = memory::cache_write(tag, index, offset, dirty, valid, this->ram[ram_address], cache_address);
            this->cycles = this->cycles + 13;

            cout << "cycle count: " << this->cycles << endl; // after every read print the # of cycles
            cout << "read cache miss stall" << endl;
            return this->ram[ram_address]; // return data we just pulled from ram
        }
    }

    else{ // don't use the cache. simply pull from ram
        int ram_address = memory::binary_int( stoll( tag + index + offset ) );
        cout << "read main memory no cache stall" << endl;
        return this->ram[ram_address];
    }

}