//
// Created by Chrystalkey on 06.07.2020.
//

#include "mouse_type.h"
#include "../FlobbelSafe.h"
#include "capturecollection.h"
#include <mutex>

HHOOK MouseCapture::mouseLLHookHandle = nullptr;
bool MouseCapture::exists = false;

MouseCapture *MouseCapture::self = nullptr;

MouseCapture::MouseCapture() {
    if(exists)
        throw instance_exists_error("MouseCapture::MouseCapture");
    infoType = FlobGlobal::MouseInfo;
    self = this;

    auto n = now();
    char date[11] = {0};
    sprintf(date,"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    clickTable = "MouseClick_"+std::string(date)+"_"+from_wstring(computerHandleStr())+"_CPACTIVITY";
    scrollTable = "MouseScroll_"+std::string(date)+"_"+from_wstring(computerHandleStr())+"_CPACTIVITY";
    moveTable = "MouseMove_"+std::string(date)+"_"+from_wstring(computerHandleStr())+"_CPACTIVITY";
    std::string table1 =
    "CREATE TABLE IF NOT EXISTS "+clickTable+"("
                                               "id INTEGER PRIMARY KEY,"
                                               "xPos INTEGER,"
                                               "yPos INTEGER,"
                                               "clickDown INTEGER,"
                                               "windowTitle TEXT,"
                                               "time INTEGER,"
                                               "button TEXT);";
    std::string table2 =
    "CREATE TABLE IF NOT EXISTS "+scrollTable+"("
                                                "id INTEGER PRIMARY KEY,"
                                                "xPos INTEGER,"
                                                "yPos INTEGER,"
                                                "windowTitle TEXT,"
                                                "time INTEGER,"
                                                "mouseDelta INTEGER);";
    std::string table3 =
    "CREATE TABLE IF NOT EXISTS "+moveTable+"("
                                              "id INTEGER PRIMARY KEY,"
                                              "xPos INTEGER,"
                                              "yPos INTEGER,"
                                              "time INTEGER);";
    FlobbelSafe::self->buildTable(table1);
    Log::self->info("MouseCapture::MouseCapture", "Table build verified: "+table1);
    FlobbelSafe::self->buildTable(table2);
    Log::self->info("MouseCapture::MouseCapture", "Table build verified: "+table2);
    FlobbelSafe::self->buildTable(table3);
    Log::self->info("MouseCapture::MouseCapture", "Table build verified: "+table3);
    CaptureCollection::self->register_threading({this, &MouseCapture::run, &MouseCapture::terminate});
    mouseLLHookHandle = SetWindowsHookExW(WH_MOUSE_LL, llMouseHook, 0, 0);
    Log::self->info("MouseCapture::MouseCapture", "Hook set");
}

MouseCapture::~MouseCapture() {
    UnhookWindowsHookEx(mouseLLHookHandle);
    Log::self->info("MouseCapture::~MouseCapture", "Hook Procedure unhooked");
}

void MouseCapture::sql_action(const Info *info) {
    if(info->infotype == FlobGlobal::MouseClick){
        std::map<MouseButton, std::wstring> buttonmap = {
                {LeftButton, L"Left_"},
                {RightButton, L"Right"},
                {MiddleButton, L"Middle"},
                {ExtraButton1, L"Extra1"},
                {ExtraButton2, L"Extra2"}
        };
        auto x = (const MouseClickInfo*)info;
        FlobbelSafe::self->insert_data(L"INSERT INTO "+from_string(clickTable)+L"(xPos, yPos, clickDown, windowTitle, time, button) VALUES(?,?,?,?,?,?);",
                              {
                                new sql_int(x->position.x),
                                new sql_int(x->position.y),
                                new sql_int(x->click_down),
                                new sql_str(x->windowTitle),
                                new sql_int(x->timestamp),
                                new sql_str(buttonmap[x->button])
                              });
    }else if(info->infotype == FlobGlobal::MouseScroll){
        auto sc = *((const MouseScrollInfo*)info);
        scrollinfoqueue.push(sc);
        if(scrollinfoqueue.size()< 25) return;
        std::wstring sql_starter = L"INSERT INTO "+from_string(scrollTable) + L"(xPos, yPos, windowTitle, time, mouseDelta) VALUES" //25 entries
                                    "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                    "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                    "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                    "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                    "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?);";
        std::deque<sql_basetype*> arguments;
        for(auto x = (const MouseScrollInfo)scrollinfoqueue.front();!scrollinfoqueue.empty();x = (const MouseScrollInfo)scrollinfoqueue.front(), scrollinfoqueue.pop()){
            arguments.push_back(new sql_int(sc.position.x));
            arguments.push_back(new sql_int(sc.position.y));
            arguments.push_back(new sql_str(sc.windowTitle));
            arguments.push_back(new sql_int(sc.timestamp));
            arguments.push_back(new sql_int(sc.mouseDelta));
        }

        FlobbelSafe::self->insert_data(sql_starter, arguments, 5);
    }else if(info->infotype == FlobGlobal::MouseMove){
        MouseMoveInfo mv = *((const MouseMoveInfo*)info);
        moveinfoqueue.push(mv);
        if(moveinfoqueue.size() < 50) return;
        std::wstring sql_starter = L"INSERT INTO "+from_string(moveTable)+L"(xPos, yPos, time) VALUES" //50 entries
                                    "(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),"
                                    "(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),"
                                    "(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),"
                                    "(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),"
                                    "(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?),(?,?,?);";
        std::deque<sql_basetype*> arguments;
        for(auto x = (const MouseMoveInfo)moveinfoqueue.front();!moveinfoqueue.empty();x = (const MouseMoveInfo)moveinfoqueue.front(), moveinfoqueue.pop()){
            arguments.push_back(new sql_int(x.pt.x));
            arguments.push_back(new sql_int(x.pt.y));
            arguments.push_back(new sql_int(x.timestamp));
        }

        FlobbelSafe::self->insert_data(sql_starter, arguments, 3);
    }else{
        Log::self->warning(L"MouseCapture::sql_action", L"Unrecognized capture Type");
    }
}

std::thread* MouseCapture::run(Capture *ths) {
    return new std::thread(&MouseCapture::evaluate, dynamic_cast<MouseCapture*>(ths));
}

std::mutex channel_lock;

LRESULT MouseCapture::llMouseHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode >= 0) {
        auto src = (MSLLHOOKSTRUCT *) lParam;
        MSLLHOOKSTRUCT msllhs = *src;
        channel_lock.lock();
        self->channelqueue.push({msllhs, wParam});
        channel_lock.unlock();
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void MouseCapture::evaluate(){
    while(!_terminate){
        Sleep(100);
        while(!channelqueue.empty()){
            channel_lock.lock();
            MouseBatch front = channelqueue.front();
            channelqueue.pop();
            channel_lock.unlock();

            DWORD wParam = front.wParam;
            auto src = (const MSLLHOOKSTRUCT*)(&(front.hs));

            if( wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP ||
                wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP ||
                wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP ||
                wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP ) {
                MouseClickInfo mci;
                mci.infotype = FlobGlobal::MouseClick;
                mci.click_down = wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN;
                mci.position = src->pt;
                mci.button = WM_LBUTTONDOWN == wParam || wParam == WM_LBUTTONUP ? LeftButton :
                             (WM_RBUTTONDOWN == wParam || wParam == WM_RBUTTONUP ? RightButton :
                              (WM_MBUTTONDOWN == wParam || wParam == WM_MBUTTONUP ? MiddleButton :
                               ((((src->mouseData) >> 16u) == 1 ? ExtraButton1 : ExtraButton2))));
                HWND temp = WindowFromPoint(src->pt);
                wchar_t array[1024] = {0};
                GetWindowTextW(temp, array, 1024);
                mci.windowTitle = std::wstring(array);

                mci.ch = FCS.handle;
                mci.timestamp = src->time;
                self->sql_action(&mci);
            }else if(wParam == WM_MOUSEMOVE){
                MouseMoveInfo mmi;
                mmi.ch = FCS.handle;
                mmi.infotype = FlobGlobal::MouseMove;
                mmi.timestamp = src->time;
                mmi.pt = src->pt;
                self->sql_action(&mmi);
            }else if( wParam == WM_MOUSEWHEEL ||
                      wParam == WM_MOUSEHWHEEL ){
                MouseScrollInfo msi;
                msi.ch = FCS.handle;
                msi.infotype = FlobGlobal::MouseScroll;
                msi.position= src->pt;
                msi.mouseDelta = ((int16_t)(src->mouseData>>16u))/120;
                msi.timestamp = src->time;
                self->sql_action(&msi);
            }
        }
    }
}