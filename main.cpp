#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <cmath>
using namespace std;

string ram[128]; // we want a 4:1 mapping from cache to DRAM - just want minimum DRAM space for demo for now. tag + index + dirty = 7 bits. data = 32 so 39 bit lines total
string cache[16]; // 2 tag + 4 index + 1 offset + 1 dirty + 1 valid + 64 (32 x 2 words per line) data = 73 bits per line x 16 lines for demo = 1168 bit cache
int cycles = 0; // count of clock cycles
int count = 0; // used to tell whether memory is handling an access
int stage = 0; // no idea, included in slides

// our choice of write policy - we have to do write-through allocate since we have cache and dram. main memory less than address space
// need logic for valid/invalid, clean/dirty cache elements
// cache 0 delay, main memory 3 delay (3 seconds, ms or somewhere between?)
// all memory timing and commands related stuff - slides
// keep a count of each W and R command - clock cycle representation in cache vs dram


// command line interface (DONE)
// cache at least "16 lines" (DONE)
// need initialization function to set all array elems to 0 - cache lines start invalid and clean (DONE)

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

int write(string addr, string data){ //respond with "wait" or "done", write to mem or cache
    if(addr.size() != 7 || data.size() != 32){
        cout << "incorrect parameter format passed to write. Try again!" << endl;
        return 0;
    }
    //parse input
    string dirty, valid; // cache only, need to get from the addr passed in by searching cache
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);

    int address = binary_int(stoll(index));
    cout << "writing to address " << address << endl;

    string new_write;

    if(cache[address][8] == '0'){
        if(offset == "0"){
            new_write = addr + "11" + data + "00000000000000000000000000000000";
        }
        else if (offset == "1"){
            new_write = addr + "11" + "00000000000000000000000000000000" + data;
        }
        cache[address] = new_write;
        cout << cache[address] << endl;
        cycles = cycles + 1;
    }
    else if(cache[address][8] == '1'){
        if(cache[address][7] == '1'){
            if(offset == "0"){
            new_write = addr + "11" + data + cache[address].substr(41, 32);
            }
            else if (offset == "1"){
                new_write = addr + "11" + cache[address].substr(9, 32) + data;
            }
            int ram_address = binary_int( stoll(cache[address].substr(0,2) + index + offset) );
            ram[ram_address] = cache[address].substr(9, 32);
            ram[ram_address+1] = cache[address].substr(41, 32);

            for(int i = 0; i < 2; i++){
                for (int j = 0; j < 2; j++){
                    cout << "wait" << endl;
                    Sleep(300);
                }
                cout<< "done" << endl;
            }
            cache[address] = new_write;
            cycles = cycles + 7;

        }
        else if(cache[address][7] == '0'){
            if(offset == "0"){
            new_write = addr + "11" + data + cache[address].substr(41, 32);
            }
            else if (offset == "1"){
                new_write = addr + "11" + cache[address].substr(9, 32) + data;
            }
            cache[address] = new_write;
            cycles = cycles + 1;
        }
    }

    return 0;
}

// addr is the address we want, memLvl tells us cache or dram (1 or 0, respectively)
string view(string addr, string memLvl){ //prints the tag, index, and offset along with the data they map to - if level is 1 for cache then also valid and dirty bits
    if(addr.size() != 7 || memLvl.size() != 1){
        cout << "incorrect parameter format passed to view. Try again!" << endl;
        return "0";
    }
    //parse input
    string dirty, valid; // cache only, need to get from the addr passed in by searching cache
    string data; //data we find from viewing
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    bool found = false;

    if(memLvl == "1"){
        int cache_address = binary_int(stoll(index));
        string line = cache[cache_address];
        if(tag == line.substr(0,2)){
            data = line;
            found = true;
        }
    }

    else{
        int ram_address = binary_int(stoll(tag + index + offset));
        found = true;
        string line = ram[ram_address];
        data = line;
    }

    if(!found && memLvl == "1"){ //couldn't find addr in cache
        return "address not found in cache!";
    }
    else if(!found && memLvl == "0"){ // addr not found in RAM, something's wrong with the addr given
        return "address not found in main RAM! Wrong format?";
    }
    else if(memLvl == "1"){ //if cache include dirty and valid bits
        return data;
    }
    else{ // final case = we asked for and found addr in main RAM
        return tag, index, offset, data;
    }
}

string read(string addr){ //respond with "wait" or "done" and return stored value
    if(addr.size() != 7){
        cout << "incorrect parameter format passed to read. Try again!" << endl;
        return 0;
    }
    //parse input
    string data; //data we find from reading, if applicable
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    bool found = false; // set to true if we find what we're trying to read

    int cache_address = binary_int(stoll(index)); //decimal version of binary cache addr for indexing 
    string line = cache[cache_address]; // the line we want to look at
    cout << "done" << endl; // 1 cycle cache access
    cycles++;
    if(tag == line.substr(0, 2)){ // found the index in the cache, now make sure the tags equal. if so, CACHE HIT
        found = true;
        if(offset == "0"){ //if offset is 0, get first piece of data (word) in line
            data = line.substr(9, 32);
        }
        else{ //if offset is 1, get second piece of data (word) in line
            data = line.substr(41, 32);
        }
    }

    if(found){ //if we had a HIT IN CACHE, return the data we read.
        return data;
    }

    else{ //else the tags didn't equal, so we have a CACHE MISS
        //find the piece of data we want to read in main memory
        string new_write;
        int ram_address = binary_int( stoll( tag + index + offset ) );
        if(offset == "0"){
            new_write = addr + "01" + ram[ram_address] + cache[cache_address].substr(41, 32);
        }
        else if (offset == "1"){
            new_write = addr + "01" + cache[cache_address].substr(9, 32) + ram[ram_address];
        }

        for(int i = 0; i < 2; i++){
            for (int j = 0; j < 2; j++){
                cout << "wait" << endl;
                Sleep(300);
            }
            cout<< "done" << endl;
            }
            cache[cache_address] = new_write;
            ram[ram_address] = cache[cache_address].substr(9, 32);
            ram[ram_address+1] = cache[cache_address].substr(41, 32);
            cycles = cycles + 7;

        return view(tag + index + offset, "1");
    }
}


int main(){
    for(int i = 0; i < 128; i++){ //initializing DRAM and cache to all 0's
        ram[i] = "00000000000000000000000000000000"; // 32 character long lines in main ram
    }
    for(int i = 0; i < 16; i++){
        cache[i] = "0000000000000000000000000000000000000000000000000000000000000000000000000"; // 73 character long lines in cache
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
            for(int i = 0; i < 128; i++){
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
    return 0;
}