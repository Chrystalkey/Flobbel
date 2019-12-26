//
// Created by Chrystalkey on 19.06.2019.
//

#include <iostream>
#include <thread>
#include <windows.h>
#include <lm.h>
#include "flobcallbackcollection.h"

ProcessCallback FlobCallbackCollection::pc = nullptr;
KeyboardCallback FlobCallbackCollection::kc = nullptr;
ScreentimeCallback FlobCallbackCollection::sc = nullptr;
HHOOK FlobCallbackCollection::keyboardHook = NULL;

FlobCallbackCollection::FlobCallbackCollection(ProcessCallback _pc, KeyboardCallback _kc, ScreentimeCallback _sc){
    pc = pc!=nullptr?pc:_pc;
    kc = kc!=nullptr?kc:_kc;
    sc = sc!=nullptr?sc:_sc;

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,llkeyhook,NULL,0);
    windowsStartup();

}

FlobCallbackCollection::~FlobCallbackCollection() {
    UnhookWindowsHookEx(keyboardHook);
    wmshutdownCallback();
}


LRESULT FlobCallbackCollection::llkeyhook(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode != HC_ACTION) return CallNextHookEx(keyboardHook,nCode,wParam,lParam);
    KeypressInfo info;
    KBDLLHOOKSTRUCT *further = (KBDLLHOOKSTRUCT*)lParam;
    std::wstring time = timestamp();

    switch(wParam){
        case WM_KEYDOWN:
            info.updown += 0;
            break;
        case WM_KEYUP:
            info.updown += 1;
            break;
        case WM_SYSKEYDOWN:
            info.updown += 2;
            break;
        case WM_SYSKEYUP:
            info.updown += 3;
            break;
        default:
            info.updown += -1;
    }
    info.scancode = (unsigned)further->scanCode;
    info.vkcode = (unsigned)further->vkCode;
    info.timestamp = time;
    wcsncpy(info.descr,map(info.vkcode).c_str(),(size_t)4);
    info.ch = FCS.handle;
    kc(info);
    return CallNextHookEx(keyboardHook,nCode,wParam,lParam);
}

void FlobCallbackCollection::run(){
    MSG msg;
    std::thread prcProcessThread(&FlobCallbackCollection::updateProcessList, this);
    while(GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    _terminate = true;
    prcProcessThread.join();
}

std::map<uint32_t, ProcessInfo> *FlobCallbackCollection::getProcessList(){
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

    do{
        ProcessInfo pinfo;
        pinfo.filename = std::wstring(pe32.szExeFile);
        if(blacklist.find(pinfo.filename) != blacklist.end()){
            if(!Process32NextW(hProcessSnap, &pe32))
                break;
            continue;
        }
        pinfo.ch = FCS.handle;
        pinfo.PID = pe32.th32ProcessID;
        pinfo.timestamp_on = timestamp();
        pinfo.timestamp_off = placeholder;
        pinfo.description = placeholder;
        pl.emplace(pinfo.PID, pinfo);
    }while(Process32NextW(hProcessSnap, &pe32));

    CloseHandle( hProcessSnap);
    return &pl;
}

void FlobCallbackCollection::updateProcessList() {
    while(!_terminate) {
        detectChanges(getProcessList());
        Sleep(100);
    }
    finishProcessList();
}

void FlobCallbackCollection::detectChanges(std::map<uint32_t, ProcessInfo> *list) {
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
            pc(x->second);
            x = processList.erase(x);
        }
        x->second.done = false;
    }
}

void FlobCallbackCollection::finishProcessList() {
    std::wstring timestmp = timestamp();
    for(auto &x: processList){
        x.second.timestamp_off = timestmp;
        pc(x.second);
    }
}

void FlobCallbackCollection::setProgramBlacklist(std::set<std::wstring> &_blacklist) {
    blacklist = _blacklist;
}
void FlobCallbackCollection::terminate(){
    PostQuitMessage(EXIT_SUCCESS);
}

void FlobCallbackCollection::windowsStartup() {
    /*
    std::chrono::milliseconds elapsed(GetTickCount());
    auto start = std::chrono::system_clock::now()-elapsed;
    time_t time = std::chrono::system_clock::to_time_t(start);
     */
    NET_API_STATUS nStatus;
    LPUSER_INFO_2 ui2 = NULL;
    WCHAR uName[256];
    DWORD uNamen = sizeof(uName);

    GetUserNameW(uName,&uNamen);
    nStatus = NetUserGetInfo(NULL,uName,2,(LPBYTE*)&ui2);
    time_t time = ui2->usri2_last_logon;
    NetApiBufferFree(ui2);
    wchar_t timestamp[20];
    struct tm *tm;
    tm = gmtime(&time);
    wsprintfW(timestamp,L"%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1,
              tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    screentimeTracker.ch = FCS.handle;
    screentimeTracker.timestamp_on = std::wstring(timestamp);
    screentimeTracker.on = time;
}

void FlobCallbackCollection::wmshutdownCallback() {
    screentimeTracker.timestamp_off = timestamp();
    auto duration = std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(screentimeTracker.on);
    auto d = std::chrono::duration_cast<std::chrono::seconds>(duration);
    screentimeTracker.duration = d.count();
    sc(screentimeTracker);
}