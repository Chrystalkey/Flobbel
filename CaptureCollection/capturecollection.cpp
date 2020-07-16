//
// Created by Chrystalkey on 19.06.2019.
//

#include <windows.h>
#include "capturecollection.h"
#include "../logging.h"

CaptureCollection::CaptureCollection() {}
void CaptureCollection::init(){
    capture_classes.emplace(FlobGlobal::Keypress, std::make_unique<KeyboardCapture>());
    FCS.log->add_capture(L"Keyboard");
    capture_classes.emplace(FlobGlobal::Process, std::make_unique<ProcessCapture>());
    FCS.log->add_capture(L"Process Uptime");
    capture_classes.emplace(FlobGlobal::Screentime, std::make_unique<ScreentimeCapture>());
    FCS.log->add_capture(L"Screentime ");
    //capture_classes.emplace(FlobGlobal::MouseInfo, std::make_unique<MouseCapture>());
    //FCS.log->add_capture(L"Mouse");
    FCS.log->finish_header();
}

void CaptureCollection::loop(){
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        for(TripleThread &tt: triplethreads){
            if(tt.cb and tt.ptr) tt.cb(tt.ptr, msg);
        }
    }
}