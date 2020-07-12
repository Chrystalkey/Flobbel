//
// Created by Chrystalkey on 06.07.2020.
//

#include "mouse_type.h"
#include "../FlobbelSafe.h"
#include "capturecollection.h"
#include <mutex>

HHOOK MouseCapture::mouseLLHookHandle = NULL;
bool MouseCapture::exists = false;

MouseCapture *MouseCapture::self = nullptr;

MouseCapture::MouseCapture() {
    if(exists)
        throw instance_exists_error("Mouse Capture");
    self = this;
    infoType = FlobGlobal::MouseInfo;
    sql_table = mouse_click_sql+mouse_scroll_sql+mouse_move_sql;
    FCS.callbackCollection->register_threading({this, &MouseCapture::run, &MouseCapture::terminate});
    mouseLLHookHandle = SetWindowsHookExW(WH_MOUSE_LL, llMouseHook, 0, 0);
}

MouseCapture::~MouseCapture() {
    UnhookWindowsHookEx(mouseLLHookHandle);
}

std::thread* MouseCapture::run(Capture *ths) {
    return new std::thread(&MouseCapture::evaluate,static_cast<MouseCapture*>(ths));
}

std::mutex mtx;
LRESULT MouseCapture::llMouseHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode < 0) return CallNextHookEx(NULL, nCode,wParam,lParam);
    auto src = (MSLLHOOKSTRUCT*)lParam;
    mtx.lock();
    self->eventQueue.emplace(MouseBatch{*src, wParam});
    mtx.unlock();
    return CallNextHookEx(NULL, nCode,wParam,lParam);
}

void MouseCapture::evaluate(){
    while(!_terminate){
        Sleep(100);

        while(!eventQueue.empty()){
            mtx.lock();
            auto src = eventQueue.front().hs;
            auto wParam = eventQueue.front().wParam;
            eventQueue.pop();
            mtx.unlock();
            if( wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP ||
                wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP ||
                wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP ||
                wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP ){
                MouseClickInfo mci;
                mci.infotype = FlobGlobal::MouseClick;
                mci.click_down = wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN;
                mci.position = src.pt;
                mci.button = WM_LBUTTONDOWN== wParam || wParam == WM_LBUTTONUP?LeftButton:
                             (WM_RBUTTONDOWN == wParam || wParam == WM_RBUTTONUP?RightButton:
                              (WM_MBUTTONDOWN == wParam || wParam == WM_MBUTTONUP?MiddleButton:
                               ((src.mouseData>>16) == 1?ExtraButton1:ExtraButton2)));
                HWND temp = WindowFromPoint(src.pt);
                DWORD winlen = GetWindowTextLengthW(temp);
                wchar_t array[winlen];
                GetWindowTextW(temp,array,winlen);
                mci.windowTitle = std::wstring(array);

                mci.ch = FCS.handle;
                mci.timestamp = src.time;
                FCS.safe->save(mci);
                continue;
            }else if( wParam == WM_MOUSEWHEEL ||
                      wParam == WM_MOUSEHWHEEL ){
                MouseScrollInfo msi;
                msi.ch = FCS.handle;
                msi.infotype = FlobGlobal::MouseScroll;
                msi.position= src.pt;
                msi.mouseDelta = ((int16_t)(src.mouseData>>16))/120;
                msi.timestamp = src.time;
                FCS.safe->save(msi);
            }else if(wParam == WM_MOUSEMOVE){
                MouseMoveInfo mmi;
                mmi.ch = FCS.handle;
                mmi.infotype = FlobGlobal::MouseMove;
                mmi.timestamp = src.time;
                FCS.safe->save(mmi);
            }else{
                Info i;
                i.ch = FCS.handle;
                i.infotype = FlobGlobal::MouseInfo;
                FCS.safe->save(i);
            }
        }
    }
}