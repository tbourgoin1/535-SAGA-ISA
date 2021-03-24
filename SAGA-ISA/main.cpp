#include <iostream>
#include <string>
#include <windows.h> //for Sleep
#include <thread> // for threading the pipe
#include <mutex> // for locking threads on memory accesses
#include <vector> // for thread list of unknown size
#include <fstream> // read text file for commands
#include <condition_variable>
#include <future>
#include <QApplication>
#include <QDir>
#include <QFile>
#include "memory.h"
#include "mainwindow.h"
#include "pipe.h"
using namespace std;

int main(int argc, char *argv[]){

   QApplication a(argc, argv);
   MainWindow w;
   w.show();
   return a.exec();
}

