//
// Created by Chrystalkey on 06.07.2020.
//

#ifndef FLOBBEL_MOUSE_TYPE_H
#define FLOBBEL_MOUSE_TYPE_H

#include <windows.h>
#include <thread>

#include "info_type.h"
#include "../global_constants.h"
#include <queue>

enum MouseButton{
    LeftButton,
    RightButton,
    MiddleButton,
    ExtraButton1,
    ExtraButton2
};

typedef struct : public Info{
    POINT position;
    bool click_down = false; //down=true;up==true
    std::wstring windowTitle;
    uint64_t timestamp;
    MouseButton button;
} MouseClickInfo;

typedef struct: public Info{
    POINT position;
    std::wstring windowTitle;
    uint64_t timestamp;
    int32_t mouseDelta = 0;
} MouseScrollInfo;

typedef struct :public Info {
    POINT pt;
    uint64_t timestamp;
} MouseMoveInfo;

typedef struct{
    MSLLHOOKSTRUCT hs;
    WPARAM wParam;
} MouseBatch;

class MouseCapture : public Capture {
public:
    MouseCapture();
    ~MouseCapture();
    static std::thread *run(Capture*ths);
    static void terminate(Capture *ths, std::thread*thr){static_cast<MouseCapture*>(ths)->_terminate=true;thr->join();delete thr;}
    void sql_action(const Info*);
private:
    static LRESULT llMouseHook(int nCode, WPARAM wParam, LPARAM lParam);

    static bool exists;
    static HHOOK mouseLLHookHandle;

    static MouseCapture* self;
private:
    std::string clickTable, scrollTable, moveTable;
    void evaluate();
    bool _terminate = false;
private:
    std::queue<MouseBatch> channelqueue;
    std::queue<MouseMoveInfo> moveinfoqueue;
    std::queue<MouseScrollInfo> scrollinfoqueue;
};

#endif //FLOBBEL_MOUSE_TYPE_H
