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
ProcessCapture* ProcessCapture::self = nullptr;

ProcessCapture::ProcessCapture() {
    if(exists)
        throw instance_exists_error("ProcessCapture::ProcessCapture");
    exists = true;
    self = this;

    auto n = now();
    char date[11] = {0};
    sprintf(date,"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    proTable = "Process_"+std::string(date)+"_"+from_wstring(computerHandleStr())+"_CPACTIVITY";

    sql_table = "CREATE TABLE IF NOT EXISTS "+proTable+"("
                "id INTEGER PRIMARY KEY,"
                "filename TEXT,"
                "description TEXT,"
                "pid INTEGER, "
                "time_on TEXT,"
                "time_off TEXT);";
    FlobbelSafe::self->buildTable(sql_table);
    Log::self->info("ProcessCapture::ProcessCapture", "Table build verified: "+sql_table);
    infoType = FlobGlobal::Process;
    CaptureCollection::self->register_threading({this, &ProcessCapture::run, &ProcessCapture::terminate, nullptr, nullptr});
}

void ProcessCapture::sql_action(const Info *info) {
    auto pack = (const ProcessInfo*)info;
    FlobbelSafe::self->insert_data(L"INSERT INTO " + from_string(proTable) + L"( filename, description, pid, time_on, time_off) VALUES(?, ?, ?, ?, ?);",
                          {
                            new sql_str(pack->filename),
                            new sql_str(pack->description),
                            new sql_int(pack->PID),
                            new sql_str(pack->timestamp_on),
                            new sql_str(pack->timestamp_off)
                          });
}

void ProcessCapture::terminate(Capture *ths, std::thread* thr){
    self->_terminate = true;
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
        Log::self->warning("ProcessCapture::getProcessList", "Failed taking snapshot of processes");
        return &pl;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if(!Process32FirstW(hProcessSnap, &pe32)){
        Log::self->warning("ProcessCapture::getProcessList", "execution of Process32First failed");
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
            pinfo.ch = FCS::computer_handle;
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
            pinfo.ch = FCS::computer_handle;
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
            sql_action(&(x->second));
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
        sql_action(&x.second);
    }
}