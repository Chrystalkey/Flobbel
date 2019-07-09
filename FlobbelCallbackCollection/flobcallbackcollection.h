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

#include "global_functions.h"
#include <map>

typedef void (*ProcessCallback)(ProcessInfo);
typedef void (*KeyboardCallback)(KeypressInfo);
typedef void (*ScreentimeCallback)(Screentime);

class FlobCallbackCollection {
public:
    FlobCallbackCollection(ProcessCallback _pc, KeyboardCallback _kc, ScreentimeCallback _sc);
    ~FlobCallbackCollection();
    void run();
    void terminate();
    void setProgramBlacklist(std::set<std::wstring> &_blacklist);

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
    std::set<std::wstring> blacklist;
    void updateProcessList();
    void detectChanges(std::map<uint32_t, ProcessInfo> *list);
    void finishProcessList();

    // Screentime
    void wmshutdownCallback();
    void windowsStartup();

    Screentime screentimeTracker;

    // Message Loop termination boolean
    bool _terminate = false;

    std::wstring placeholder = L"platzhalterwobbel";
};


#endif //FLOBBEL_FLOBCALLBACKCOLLECTION_H
