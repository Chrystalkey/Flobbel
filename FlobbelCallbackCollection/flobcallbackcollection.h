//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_FLOBCALLBACKCOLLECTION_H
#define FLOBBEL_FLOBCALLBACKCOLLECTION_H

#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <vector>
#include <set>

#include "../global_functions.h"
#include <map>

typedef struct{
    ComputerHandle ch;
    std::string filename;
    std::string description;
    std::string timestamp_on;
    std::string timestamp_off;
    uint32_t PID = 0;
    bool done = false;
} ProcessInfo;

typedef struct{
    ComputerHandle ch;
    uint8_t updown = 0; // down == updown%2 == 0; up == updown%2 == 1
    uint32_t scancode = 0;
    uint32_t vkcode = 0;
    char descr[4] = {0};
    std::string timestamp;
} KeypressInfo;

typedef struct{
    ComputerHandle ch;
    std::string timestamp_on;
    std::string timestamp_off;
} Screentime;

typedef void (*ProcessCallback)(ProcessInfo);
typedef void (*KeyboardCallback)(KeypressInfo);
typedef void (*ScreentimeCallback)(Screentime);

class FlobCallbackCollection {
public:
    FlobCallbackCollection(ProcessCallback _pc, KeyboardCallback _kc, ScreentimeCallback _sc);
    ~FlobCallbackCollection();
    void run();
    void terminate();
    void setProgramBlacklist(std::set<std::string> &_blacklist);

private:
    // Callbacks
    static ProcessCallback pc;
    static KeyboardCallback kc;
    static ScreentimeCallback sc;

    // Keyboard
    static LRESULT llkeyhook(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK keyboardHook;

    // Processes
    std::map<uint32_t, ProcessInfo> *getProcessList();
    std::map<uint32_t, ProcessInfo> processList;
    std::set<std::string> blacklist;
    void updateProcessList();
    void detectChanges(std::map<uint32_t, ProcessInfo> *list);
    void finishProcessList();

    // Message Loop termination boolean
    bool _terminate = false;

    std::string placeholder = "platzhalterwobbel";
};


#endif //FLOBBEL_FLOBCALLBACKCOLLECTION_H
