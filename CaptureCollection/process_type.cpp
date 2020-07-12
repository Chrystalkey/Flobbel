//
// Created by Chrystalkey on 02.07.2020.
//

#include <windows.h>
#include <tlhelp32.h>
//#include <tchar.h>

#include "../global_functions.h"
#include "process_type.h"
#include "capturecollection.h"
#include "../FlobbelSafe.h"

bool ProcessCapture::exists = false;

ProcessCapture::ProcessCapture() {
    if(exists)
        throw instance_exists_error("ProcessCapture");
    exists = true;
    infoType = FlobGlobal::Process;
    FCS.callbackCollection->register_threading({this, &ProcessCapture::run, &ProcessCapture::terminate, nullptr, nullptr});
}

void ProcessCapture::terminate(Capture *ths, std::thread* thr){
    static_cast<ProcessCapture*>(ths)->_terminate = true;
    thr->join();
    delete thr;
}
std::thread* ProcessCapture::run(Capture *ths){
    return new std::thread(&ProcessCapture::updateProcessList, static_cast<ProcessCapture*>(ths));
}
std::map<uint32_t, ProcessInfo> *ProcessCapture::getProcessList(){
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32;
    static std::map<uint32_t, ProcessInfo> pl; // PID, pi
    pl.clear();

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessSnap == INVALID_HANDLE_VALUE){
        std::wcerr << L"ERROR while taking snapshot of processes\n";
        return &pl;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if(!Process32FirstW(hProcessSnap, &pe32)){
        std::wcerr << L"ERROR while executing Process32First\n";
        CloseHandle(hProcessSnap);
        return &pl;
    }

    if(runsAtPrgmStart){
        do{
            ProcessInfo pinfo;
            pinfo.filename = std::wstring(pe32.szExeFile);
            if(blacklist.find(pinfo.filename) != blacklist.end()){
                if(!Process32NextW(hProcessSnap, &pe32))
                    break;
                continue;
            }
            pinfo.ch = FCS.handle;
            pinfo.infotype = infoType;
            pinfo.PID = pe32.th32ProcessID;
            pinfo.timestamp_on = timestamp();
            pinfo.timestamp_off = placeholder;
            pinfo.description = placeholder;
            pinfo.execAtPrgmStart = true;
            pl.emplace(pinfo.PID, pinfo);
        }while(Process32NextW(hProcessSnap, &pe32));
    }
    else{
        do{
            ProcessInfo pinfo;
            pinfo.filename = std::wstring(pe32.szExeFile);
            if(blacklist.find(pinfo.filename) != blacklist.end()){
                if(!Process32NextW(hProcessSnap, &pe32))
                    break;
                continue;
            }
            pinfo.ch = FCS.handle;
            pinfo.infotype = infoType;
            pinfo.PID = pe32.th32ProcessID;
            pinfo.timestamp_on = timestamp();
            pinfo.timestamp_off = placeholder;
            pinfo.description = placeholder;
            pl.emplace(pinfo.PID, pinfo);
        }while(Process32NextW(hProcessSnap, &pe32));
    }

    CloseHandle( hProcessSnap);
    return &pl;
}

void ProcessCapture::updateProcessList() {
    while(!_terminate) {
        detectChanges(getProcessList());
        Sleep(100);
    }
    finishProcessList();
}

void ProcessCapture::detectChanges(std::map<uint32_t, ProcessInfo> *list) {
    for(auto &x : *list){
        auto running_process = processList.find(x.first);
        if(running_process == processList.end()){
            processList.emplace(x.first,x.second);
        }
        processList[x.first].done = true;
    }
    for(auto x = processList.begin(); x != processList.end(); x++){
        if(!x->second.done){
            x->second.timestamp_off = timestamp();
            FCS.safe->save(x->second);
            x = processList.erase(x);
        }
        x->second.done = false;
    }
}

void ProcessCapture::finishProcessList() {
    std::wstring timestmp = timestamp();
    for(auto &x: processList){
        x.second.timestamp_off = timestmp;
        x.second.execAtPrgmStop = true;
        FCS.safe->save(x.second);
    }
}