//
// Created by Chrystalkey on 19.06.2019.
//

#include <windows.h>
#include <shlwapi.h>
#include "capturecollection.h"

void exitWatchdog();

CaptureCollection::CaptureCollection() {exitWatchdogThread = new std::thread(&::exitWatchdog);}
CaptureCollection::~CaptureCollection(){
    exitWatchdogThread->join();
    delete exitWatchdogThread;
}

void exitWatchdog(){
    while(!PathFileExistsW(L"exit")) Sleep(500);
    Log::self->warning("exitWatchdog", "Exit Watchdog encountered exit file. Now Terminating.");
    DeleteFileW(L"exit");
    RemoveDirectoryW(L"exit");
    PostThreadMessageW(FCS::mainthread_id, WM_QUIT, 0, 0);
}

CaptureCollection *CaptureCollection::self = nullptr;
void CaptureCollection::init(){
    if(self) throw instance_exists_error("CaptureCollection::init");
    self = new CaptureCollection;
    self->capture_classes.emplace(FlobGlobal::Keypress, std::make_shared<KeyboardCapture>());
    Log::self->add_capture(L"Keyboard");
    self->capture_classes.emplace(FlobGlobal::Process, std::make_shared<ProcessCapture>());
    Log::self->add_capture(L"Process Uptime");
    self->capture_classes.emplace(FlobGlobal::Screentime, std::make_shared<ScreentimeCapture>());
    Log::self->add_capture(L"Screentime ");
    self->capture_classes.emplace(FlobGlobal::MouseInfo, std::make_shared<MouseCapture>());
    Log::self->add_capture(L"Mouse");
    Log::self->finish_header();
}

void CaptureCollection::loop(bool (*callback)(const MSG&)){
    MSG msg;
    Log::self->info("CaptureCollection::loop", "Message loop on main thread initiated.");
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if(callback(msg)){
            PostQuitMessage(0);
        }
        for(TripleThread &tt: triplethreads){
            if(tt.cb and tt.ptr) tt.cb(tt.ptr, msg);
        }
    }
}