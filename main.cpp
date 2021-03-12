#include <iostream>
#include <string>
using namespace std;

int ram[128000000];
int cache[100];

int write(){
    cout << "write command" << endl;
    return 0;
}

int read(){
    cout << "read command" << endl;
    return 0;
}

int view(){
    cout << "view command" << endl;
    return 0;
}

int main(){
    while(1){
        string input;
        cin >> input;
        if(input == "w"){
            write();
        }
        else if(input == "r"){
            read();
        }
        else if(input == "v"){
            view();
        }
        else if(input == "exit"){
            cout << "exiting program..." << endl;
            break;
        }
        else{
            cout << "please enter a valid command!" << endl;
        }
    }
    return 0;
}