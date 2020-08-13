//
// Created by Chrystalkey on 02.07.2020.
//

#ifndef FLOBBEL_KEYPRESS_TYPE_H
#define FLOBBEL_KEYPRESS_TYPE_H

#include "../global_constants.h"
#include "info_type.h"
#include "windows.h"
#include <queue>

typedef struct:public Info{
    uint8_t clickdown = 0; // down == clickdown%2 == 0; up == clickdown%2 == 1
    uint32_t scancode = 0;
    uint32_t vkcode = 0;
#ifdef __WIN32__
    wchar_t descr[5] = {0};
    time_t timestamp;
#else
    char descr[5] = {0};
    std::string timestamp;
#endif
} KeypressInfo;

class KeyboardCapture: public Capture {
public:
    explicit KeyboardCapture();
    ~KeyboardCapture() override;
    void sql_action(const Info*) override;
    static void terminate();
private:
    std::string keyTable;

    std::queue<KeypressInfo> accumulation_queue;

    static LRESULT llkeyhook(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;
    static bool exists;
    static KeyboardCapture* self;
};

#endif //FLOBBEL_KEYPRESS_TYPE_H
