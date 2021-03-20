#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <cmath>
#include "memory.h"
using namespace std;

/**
int count = 0; // used to tell whether memory is handling an access
int stage = 0; // no idea, included in slides

//converts binary to integer value
int binary_int(long long n){
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

string cache_write(string tag, string index, string offset, string dirty, string valid, string data, int cache_address){
    string new_write;
    if(offset == "00") //if 00 then write new data to 1st cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + data + cache[cache_address].substr(42, 32) + cache[cache_address].substr(74, 32) + cache[cache_address].substr(106, 32);
    else if(offset == "01") //if 01 then write new data to 2nd cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + cache[cache_address].substr(10, 32) + data + cache[cache_address].substr(74, 32) + cache[cache_address].substr(106, 32);
    else if(offset == "10") //if 10 then write new data to 3rd cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + cache[cache_address].substr(10, 32) + cache[cache_address].substr(42, 32) + data + cache[cache_address].substr(106, 32);
    else if(offset == "11") //if 11 then write new data to 4th cache slot for this line and copy same old data from before 
        new_write = tag + index + offset + dirty + valid + cache[cache_address].substr(10, 32) + cache[cache_address].substr(42, 32) + cache[cache_address].substr(74, 32) + data;
    return new_write;
}

int write(string addr, string data){ //respond with "wait" or "done", write to mem or cache
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
    int address = binary_int(stoll(index));
    
    cout << "writing to address " << address << endl;
    
    if(cache[address][9] == '0'){ // checking valid bit - invalid case. Can just write as it's empty.
        cache[address] = cache_write(tag, index, offset, dirty, valid, data, address);
        cycles++;
    }
    else if(cache[address][9] == '1'){ // checking valid bit - valid case. Need to do additional checks as it's been written to before
        if(cache[address][8] == '1'){ // check dirty bit, if 1 then need to write back to ram before writing to cache
            // write back all 4 words in cache line to memory
            int ram_address = binary_int( stoll(cache[address].substr(0,2) + index + offset) );
            ram[ram_address] = cache[address].substr(10, 32); // data1
            ram[ram_address+1] = cache[address].substr(42, 32); // data2
            ram[ram_address+2] = cache[address].substr(74, 32); // data3
            ram[ram_address+3] = cache[address].substr(106, 32); // data4
            for(int i = 0; i < 4; i++){ // simulate 4 memory accesses
                for (int j = 0; j < 2; j++){
                    cout << "wait" << endl;
                    Sleep(300);
                }
                cout<< "done" << endl;
            }
            cycles = cycles + 12; // (4 * 3) = 12 cycles for 4 memory accesses
        }
        cache[address] = cache_write(tag, index, offset, dirty, valid, data, address);
        cycles++; // +1 for cache access
    }

    cout << "cycle count: " << cycles << endl; // after every write print the # of cycles
    return 0;
}

// addr is the address we want, memLvl tells us cache or dram (1 or 0, respectively)
string view(string addr, string memLvl){ //prints the tag, index, and offset along with the data they map to - if level is 1 for cache then also valid and dirty bits
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
        int cache_address = binary_int(stoll(index));
        string line = cache[cache_address];
        if(tag == line.substr(0,2)){
            dirty = line.substr(8, 1);
            valid = line.substr(9, 1);
            data = line.substr(10, 32) + " " + line.substr(42, 32) + " " + line.substr(74, 32) + " " + line.substr(106, 32);
            found = true;
        }
    }

    else{ // case to read from memory. Should always find a data value
        int ram_address = binary_int(stoll(tag + index + offset));
        found = true;
        string line = ram[ram_address];
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

string read(string addr){ //respond with "wait" or "done" and return stored value
    if(addr.size() != 8){ // address size must be 8
        cout << "incorrect parameter format passed to read. Try again!" << endl;
        return 0;
    }
    //parse input
    string data; //data we find from reading, if applicable
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 2);
    string dirty = addr.substr(8, 1);
    string valid = addr.substr(9, 1);
    int cache_address = binary_int(stoll(index)); //decimal version of binary cache addr for indexing 

    if(valid == "0") {
        int ram_address = binary_int( stoll( tag + index + "00" ) );
        string new_write = tag + index + offset + dirty + valid + ram[ram_address] + ram[ram_address+1] + ram[ram_address+2] + ram[ram_address+3];
        cache[cache_address] = new_write;
    }

    string line = cache[cache_address]; // the line of cache we want to initially look at (matches index)
    cout << "done" << endl; // 1 cycle cache access
    cycles++;
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
        int ram_address = binary_int( stoll( tag + index + offset ) );
        string dirty = "0";
        string valid = "1";
        for(int i = 0; i < 2; i++){
            for (int j = 0; j < 2; j++){
                cout << "wait" << endl;
                Sleep(300);
            }
            cout<< "done" << endl;
            }
            string old_tag = cache[cache_address].substr(0, 2);
            int old_ram_address = binary_int(stoll(old_tag + index + "00"));
            for(int i = 0; i < 4; i++){ // write back to ram
                ram[old_ram_address + i] = cache[cache_address].substr(10 + (32 * i), 32);
            }
            cache[cache_address] = cache_write(tag, index, offset, dirty, valid, ram[ram_address], cache_address);
            cycles = cycles + 7;

        cout << "cycle count: " << cycles << endl; // after every read print the # of cycles
        return ram[ram_address]; // return data we just pulled from ram
    }

}
**/

int main(){
    /**
    for(int i = 0; i < 256; i++){ //initializing DRAM and cache to all 0's
        ram[i] = "00000000000000000000000000000000"; // 32 character long lines in main ram
    }
    for(int i = 0; i < 16; i++){
        cache[i] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"; // 138 character long lines in cache
    }
    while(1){
        string command, param1, param2; // command for w, r, or v, 1st parameter for command, 2nd parameter for command, not present with r
        cin >> command; //read in command and test
        if(command == "w"){
            cin >> param1;
            cin >> param2;
            write(param1, param2);
        }
        else if(command == "r"){
            cin >> param1;
            cout << "read returned: \n" << read(param1) << endl;
        }
        else if(command == "v"){
            cin >> param1;
            cin >> param2;
            cout << "view returned: \n" << view(param1, param2) << endl;
        }
        else if(command == "exit"){
            cout << "exiting program..." << endl;
            break;
        }
        else if(command == "cache"){
            for(int i = 0; i < 16; i++){
                cout << cache[i] << endl;
            }
        }
        else if(command == "ram"){
            for(int i = 0; i < 256; i++){
                cout << ram[i] << endl;
            }
        }
        else if(command == "cycles"){
            cout << "no. of cycles: " << cycles << endl;
        }
        else{
            cout << "please enter a valid input!" << endl;
        }
    }
    **/
    cout << "test" << endl;
    memory mem;
    cout << "cycles: " << mem.get_cycles() << endl;
    for(int i = 0; i < 256; i++){
        cout<< mem.get_ram()[i] << endl;
    }
    return 0;
}