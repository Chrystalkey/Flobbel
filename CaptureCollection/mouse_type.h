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

static const std::string mouse_click_sql = "CREATE TABLE IF NOT EXISTS MouseClick("
                              "id INTEGER PRIMARY KEY,"
                              "xPos INTEGER,"
                              "yPos INTEGER,"
                              "clickDown INTEGER,"
                              "windowTitle TEXT,"
                              "time INTEGER,"
                              "button TEXT);";
static const std::string mouse_scroll_sql = "CREATE TABLE IF NOT EXISTS MouseScroll("
                               "id INTEGER PRIMARY KEY,"
                               "xPos INTEGER,"
                               "yPos INTEGER,"
                               "windowTitle TEXT,"
                               "time INTEGER,"
                               "mouseDelta INTEGER);";
static const std::string mouse_move_sql = "CREATE TABLE IF NOT EXISTS MouseMove("
                             "id INTEGER PRIMARY KEY,"
                             "xPos INTEGER,"
                             "yPos INTEGER,"
                             "time INTEGER);";

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

struct MouseBatch{
    MSLLHOOKSTRUCT hs;
    WPARAM wParam;
};

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
    std::queue<struct MouseBatch> eventQueue;
    void evaluate();
    bool _terminate = false;
};

#endif //FLOBBEL_MOUSE_TYPE_H
