//
// Created by Chrystalkey on 01.07.2019.
//

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <windows.h>
#include "FlobbelSafe.h"

FlobbelSafe::FlobbelSafe(std::wstring _safedir):safedir(_safedir) {
    //Screentime
    screenOut.open(converter.to_bytes(safedir+L"SCREENTIME;"+std::wstring(computerHandleStr())+L";CPACTIVITY.flob"), std::ios::out|std::ios::app);
    if(!screenOut.is_open()){
        std::wcerr << L"ERROR opening " << L"SCREENTIME;"+std::wstring(computerHandleStr())+L";CPACTIVITY.flob\n";
    }

    wchar_t date[11];
    auto n = now();
    wsprintfW(date,L"%02d.%02d.%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    //keys
    keyOut.open(converter.to_bytes(safedir+L"KEYS;"+std::wstring(date)+L";"+std::wstring(computerHandleStr())+L";CPACTIVITY.flob"), std::ios::out|std::ios::app);
    if(!keyOut.is_open()){
        std::wcerr << L"ERROR opening " << L"KEYS;"+std::wstring(date)+L";"+std::wstring(computerHandleStr())+L";CPACTIVITY.flob\n";
    }

    //processes
    procOut.open(converter.to_bytes(safedir+L"PROC;"+std::wstring(date)+L";"+std::wstring(computerHandleStr())+L";CPACTIVITY.flob"), std::ios::out|std::ios::app);
    if(!procOut.is_open()){
        std::wcerr << L"ERROR opening " << L"PROC;"+std::wstring(date)+L";"+std::wstring(computerHandleStr())+L";CPACTIVITY.flob\n";
    }
    keyOut  << L"<<<New Program Startup---" << timestamp() << L">>>\n";
    procOut << L"<<<New Program Startup---" << timestamp() << L">>>\n";
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
    wchar_t line[128];
    wsprintfW(line,L"%c;%08x;%08x;%s;%s\n",info.updown%2?L'u':L'd',info.vkcode,info.scancode,info.descr,info.timestamp.c_str());
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
    std::wstringstream s;
    s << std::setw(8) << std::setfill(L'0') << info.PID << L";" << info.timestamp_on
      << L";" << info.timestamp_off << L"|" << info.filename << L"\n";
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
    wchar_t buffer[128];
    wsprintfW(buffer, L"%s|%s|%s|%s\n", computerHandleStr().c_str(),info.timestamp_on.c_str(), info.timestamp_off.c_str(),info.duration.c_str());
    screenOut << buffer;

}