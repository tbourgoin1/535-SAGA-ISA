#include <iostream>
#include <string>
#include "memory.h"
using namespace std;

int count = 0; // used to tell whether memory is handling an access
int stage = 0; // no idea, included in slides

int main(){
    cout << "test" << endl;
    memory mem;
    while(1){
        string command, param1, param2; // command for w, r, or v, 1st parameter for command, 2nd parameter for command, not present with r
        cin >> command; //read in command and test
        if(command == "w"){
            cin >> param1;
            cin >> param2;
            mem.write(param1, param2);
        }
        else if(command == "r"){
            cin >> param1;
            cout << "read returned: \n" << mem.read(param1) << endl;
        }
        else if(command == "v"){
            cin >> param1;
            cin >> param2;
            cout << "view returned: \n" << mem.view(param1, param2) << endl;
        }
        else if(command == "exit"){
            cout << "exiting program..." << endl;
            break;
        }
        else if(command == "cache"){
            for(int i = 0; i < 16; i++){
                cout<< mem.get_cache()[i] << endl;
            }
        }
        else if(command == "ram"){
            for(int i = 0; i < 256; i++){
                cout<< mem.get_ram()[i] << endl;
            }
        }
        else if(command == "cycles"){
            cout<< mem.get_cycles() << endl;
        }
        else{
            cout << "please enter a valid input!" << endl;
        }
    }
}