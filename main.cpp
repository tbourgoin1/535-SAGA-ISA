#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <cmath>
using namespace std;

string ram[64]; // we want a 4:1 mapping from cache to DRAM - just want minimum DRAM space for demo for now. tag + index + dirty = 7 bits. data = 32 so 39 bit lines total
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
    cout << "address " << address << endl;

    string new_write;
    if(offset == "0"){
        new_write = addr + "11" + data + "00000000000000000000000000000000";
    }
    else if (offset == "1"){
        new_write = addr + "11" + "00000000000000000000000000000000" + data;
    }

    cout << "new_write " << new_write << endl;

    if(cache[address][8] == '0'){
        cout << "WRITING..." << endl;
        cache[address] = new_write;
        cout << cache[address] << endl;
        cycles = cycles + 1;
    }
    else if(cache[address][8] == '1'){
        if(cache[address][7] == '1'){
            int ram_address = binary_int( stoll(tag + index + "0") );
            ram[ram_address] = new_write.substr(9, 32);
            ram[ram_address+1] = new_write.substr(41, 32);

            for(int i = 0; i < 2; i++){
                for (int j = 0; j < 2; j++){
                    cout << "wait";
                }
                cout<< "done";
            }
            cache[address] = new_write;
            cycles = cycles + 7;

        }
        else if(cache[address][7] == '1'){
            cache[address] = new_write;
            cycles = cycles + 1;
        }
    }

    return 0;
}


string read(string addr){ //respond with "wait" or "done" and return stored value
    if(addr.size() != 7){
        cout << "incorrect parameter format passed to read. Try again!" << endl;
        return 0;
    }
    //parse input
    string data; //data we find from reading, if applicable
    string data2; //if we need to write back to cache, this will be the 2nd word for when offset = 1
    string tag = addr.substr(0, 2);
    string index = addr.substr(2, 4);
    string offset = addr.substr(6, 1);
    string whichData;
    if(offset == "0"){ //which data to return depending on offset? data or data2?
        whichData = "1";
    }
    else{
        whichData = "2";
    }
    bool found = false; // set to true if we find what we're trying to read
    bool needsEviction = true; // used to identify if we need to replace something in the cache (cache is full & we had a cache miss when reading)

    for(int i = 0; i < 16; i++){ // search the cache first
        string line = cache[i]; // gets full 73 char string for line
        if(index == line.substr(2, 4)){ //if index is found, check tag
            if(tag == line.substr(0, 2)){ // if the tag matches too then we found the addr in cache - READ HIT. get the dirty and valid bits along with the data for the appropriate offset.
                found = true;
                if(offset == "0"){ //if offset is 0, get first piece of data (word) in line
                    data = line.substr(9, 32);
                }
                else{ //if offset is 1, get second piece of data (word) in line
                    data = line.substr(41, 32);
                }
            }
        }
    }
    if(found){ //if we had a HIT IN CACHE, return the data we read.
        return data;
    }
    else{ //else we had a miss in the cache, go look for the address in main memory and write back to cache
        for(int i = 0; i < 64; i++){
            string line = ram[i]; // gets full 39 char string for line
            if(index == line.substr(2, 4)){ //if index is found, check tag
                if(tag == line.substr(0, 2)){ // if the tag matches too, we found the right address. we just need BOTH words for the cache (offset 0 or 1)
                    if(offset == "0"){ // finally if the offset is 0 or 1, take its data. 0 will be data and 1 will be data2
                        found = true;
                        data = line.substr(7, 32); // get data for the address
                    }
                    else if(offset == "1"){
                        found = true;
                        data2 = line.substr(7, 32);
                    }
                }
            }
        }
    }
    if(!found){
        return "Something went wrong in read. It couldn't find the data for the address given in cache OR main memory.";
    }
    else{ //now that we've found it, we need to write this data back to the cache and make sure to check for dirty bits
        for(int i = 0; i < 16; i++){ // search the cache for an empty line
            string line = cache[i]; // gets full 73 char string for line
            if(line.substr(8, 1) == "0"){ //if the valid bit is not set (therefore invalid), it's an empty line and we can freely write to it. valid bit = 1, dirty bit = 0
                needsEviction = false; // found empty line, don't need to replace anything
                cache[i] = tag + index + "0" + "0" + "1" + data + data2; // write tag, index, offset, dirty bit, valid bit, both words (for each offset) to cache. keep the offset as 0 always
            }
        }
        if(needsEviction){ // the cache is full, need to evict a member of it
            for(int i = 0; i < 16; i++){ // search the cache for the index of our new line we're inserting into the cache
                string line = cache[i]; // gets full 73 char string for line
                if(index == line.substr(2, 4)){ // when we find the matching index, evict
                    if(line.substr(7, 1) == "0"){ // not dirty, so just replace old cache line with our new one
                        cache[i] = tag + index + "0" + "0" + "1" + data + data2; // write tag, index, offset, dirty bit, valid bit, both words (for each offset) to cache. keep the offset as 0 always
                        break; //don't need to look anymore
                    }
                    else if(line.substr(7, 1) == "1"){ //dirty, need to write both words from current cache line back to main memory
                        for(int i = 0; i < 64; i++){ // search ram for the index
                            string line = ram[i]; // gets full 39 char string for line
                            if(index == line.substr(2, 4)){ //if index is found, check tag
                                if(tag == line.substr(0, 2)){ // if the tag matches too, we found the right address. we just need to write a diff value depending on offset
                                    if(line.substr(6, 1) == "0"){ // if offset is 0, write the first 32 bits of the data part of the cache to main memory
                                        ram[i] = tag + index + data;
                                    }
                                    else if(line.substr(6, 1) == "1"){ // if offset is 1, write the second 32 bits of the data part of the cache to main memory
                                        ram[i] = tag + index + data2;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(whichData == "1"){
        return data;
    }
    else if(whichData == "2"){
        return data2;
    }
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
    /**
    if(memLvl == "1"){ //try to find given addr in cache
        for(int i = 0; i < 16; i++){
            string line = cache[i]; // gets full 73 char string for line
            if(index == line.substr(2, 4)){ //if index is found, check tag
                if(tag == line.substr(0, 2)){ // if the tag matches too then we found the addr in cache. get the dirty and valid bits along with the data for the appropriate offset.
                    found = true;
                    dirty = line.substr(7, 1); // get dirty bit from line
                    valid = line.substr(8, 1); // get valid bit from line
                    if(offset == "0"){ //if offset is 0, get first piece of data (word) in line
                        data = line.substr(9, 32);
                    }
                    else{ //if offset is 1, get second piece of data (word) in line
                        data = line.substr(41, 32);
                    }
                }
            }
        }
    }
    **/
    if(memLvl == "1"){
        int cache_address = binary_int(stoll(index));
        string line = cache[cache_address];
        if(tag == line.substr(0,2)){
            found = true;
            dirty = line.substr(7, 1); // get dirty bit from line
            valid = line.substr(8, 1); // get valid bit from line
            if(offset == "0"){ //if offset is 0, get first piece of data (word) in line
                data = line.substr(9, 32);
            }
            else{ //if offset is 1, get second piece of data (word) in line
                data = line.substr(41, 32);
            }
        }
    }
    /**
    else{ //try to find in main ram
        for(int i = 0; i < 64; i++){
            string line = ram[i]; // gets full 39 char string for line
            if(index == line.substr(2, 4)){ //if index is found, check tag
                if(tag == line.substr(0, 2)){ // if the tag matches too, check the last bit - offset.
                    if(offset == line.substr(6, 1)){ // finally if the offset matches, we can say we found the data in main ram
                        found = true;
                        data = line.substr(7, 32); // get data for the address
                    }
                }
            }
        }
    }
    **/
    else{
        int ram_address = binary_int(stoll(tag + index));
        string line = ram[ram_address];
    }

    if(!found && memLvl == "1"){ //couldn't find addr in cache
        return "address not found in cache!";
    }
    else if(!found && memLvl == "0"){ // addr not found in RAM, something's wrong with the addr given
        return "address not found in main RAM! Wrong format?";
    }
    else if(memLvl == "1"){ //if cache include dirty and valid bits
        return tag, index, offset, dirty, valid, data; //needs to be formatted
    }
    else{ // final case = we asked for and found addr in main RAM
        return tag, index, offset, line;
    }
}

int main(){
    for(int i = 0; i < 64; i++){ //initializing DRAM and cache to all 0's
        ram[i] = "000000000000000000000000000000000000000"; // 39 character long lines in main ram
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
            cout << "write returned: \n" << write(param1, param2) << endl;
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
        else{
            cout << "please enter a valid input!" << endl;
        }
    }
    return 0;
}