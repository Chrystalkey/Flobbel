//
// Created by Chrystalkey on 01.07.2019.
//

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <windows.h>
#include "FlobbelSafe.h"

FlobbelSafe::FlobbelSafe(std::string _safedir):safedir(_safedir) {
    //Screentime
    screenOut.open(safedir+"SCREENTIME;"+std::string(computerHandleStr())+";CPACTIVITY.flob", std::ios::out|std::ios::app);
    if(!screenOut.is_open()){
        std::cerr << "ERROR opening " << "SCREENTIME;"+std::string(computerHandleStr())+";CPACTIVITY.flob\n";
    }

    char date[11];
    auto n = now();
    sprintf(date,"%02d.%02d.%04d",n->tm_mday, n->tm_mon, n->tm_year+1900);
    //keys
    keyOut.open(safedir+"KEYS;"+std::string(date)+";"+std::string(computerHandleStr())+";CPACTIVITY.flob", std::ios::out|std::ios::app);
    if(!keyOut.is_open()){
        std::cerr << "ERROR opening " << "KEYS;"+std::string(date)+";"+std::string(computerHandleStr())+";CPACTIVITY.flob\n";
    }

    //processes
    procOut.open(safedir+"PROC;"+std::string(date)+";"+std::string(computerHandleStr())+";CPACTIVITY.flob", std::ios::out|std::ios::app);
    if(!procOut.is_open()){
        std::cerr << "ERROR opening " << "PROC;"+std::string(date)+";"+std::string(computerHandleStr())+";CPACTIVITY.flob\n";
    }
}

FlobbelSafe::~FlobbelSafe() {
    finalize_files();
}

void FlobbelSafe::finalize_files() {
    while(!prcQueue.empty()){
        procOut << prcQueue.front();
        prcQueue.pop();
    }
    while(!keyQueue.empty()){
        keyOut << keyQueue.front();
        keyQueue.pop();
    }
    screenOut.close();
    keyOut.close();
    procOut.close();
}

void FlobbelSafe::add_key(KeypressInfo &info) {
    if(!keyOut.is_open()) return;
    char line[128];
    sprintf(line,"%c;%08x;%08x;%s;%s\n",info.updown%2?'u':'d',info.vkcode,info.scancode,info.descr,info.timestamp.c_str());
    keyQueue.push(line);
    if(keyQueue.size() == 20){
        while(keyQueue.size() >0){
            keyOut << keyQueue.front();
            keyQueue.pop();
        }
    }
}

void FlobbelSafe::add_prc(ProcessInfo &info) {
    if(!procOut.is_open()) return;
    std::stringstream s;
    s << std::setw(8) << std::setfill('0') << info.PID << ";" << info.timestamp_on
      << ";" << info.timestamp_off << "|" << info.filename << "\n";
    prcQueue.push(s.str());
    if(prcQueue.size() == 10){
        while(prcQueue.size() >0){
            procOut << prcQueue.front();
            prcQueue.pop();
        }
    }
}

void FlobbelSafe::add_screentime(Screentime &info) {
    if(!screenOut.is_open()) return;
    char buffer[128];
    sprintf(buffer, "%s|%s|%s|%s\n", computerHandleStr().c_str(),info.timestamp_on.c_str(), info.timestamp_off.c_str(),info.duration.c_str());
    screenOut << buffer;

}