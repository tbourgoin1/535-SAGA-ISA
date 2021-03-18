#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <cmath>
using namespace std;

string ram[128]; // 4:1 ram:cache direct mapping with offset
string cache[16]; // bits | tag: 2 | index: 4 | offset: 1 | valid: 1 | dirty: 1 | data1: 32 | data2: 32
int cycles = 0; // count of clock cycles
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
    if(offset == "0")
        new_write = tag + index + offset + dirty + valid + data + cache[cache_address].substr(41, 32);
    else if (offset == "1")
        new_write = tag + index + offset + dirty + valid + cache[cache_address].substr(9, 32) + data;
    return new_write;
}

int write(string addr, string data){ //respond with "wait" or "done", write to mem or cache
    if(addr.size() != 7 || data.size() != 32){
        cout << "incorrect parameter format passed to write. Try again!" << endl;
        return 1;
    }
    //parse input
    string dirty = "1"; // writing to cache will always activate dirty bit
    string valid = "1";
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    int address = binary_int(stoll(index));
    
    cout << "writing to address " << address << endl;
    
    if(cache[address][7] == '0'){
        cache[address] = cache_write(tag, index, offset, dirty, valid, data, address);
        cycles++;
    }
    else if(cache[address][7] == '1'){
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
        cache[address] = cache_write(tag, index, offset, dirty, valid, data, address);;
        cycles = cycles + 7;
    }

    cout << "cycle count: " << cycles << endl; // after every write print the # of cycles
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

    int cache_address = binary_int(stoll(index)); //decimal version of binary cache addr for indexing 
    string line = cache[cache_address]; // the line we want to look at
    cout << "done" << endl; // 1 cycle cache access
    cycles++;
    if(tag == line.substr(0, 2)){ // found the index in the cache, now make sure the tags equal. if so, CACHE HIT
        if(offset == "0") //if offset is 0, get first piece of data (word) in line
            data = line.substr(9, 32);
        else //if offset is 1, get second piece of data (word) in line
            data = line.substr(41, 32);
        return data; // cache hit
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
            cache[cache_address] = cache_write(tag, index, offset, dirty, valid, ram[ram_address], cache_address);
            cycles = cycles + 7;

        cout << "cycle count: " << cycles << endl; // after every read print the # of cycles
        return ram[ram_address]; // return data we just pulled from ram
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