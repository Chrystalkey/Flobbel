//
// Created by Chrystalkey on 02.07.2020.
//

#ifndef FLOBBEL_SCREENTIME_TYPE_H
#define FLOBBEL_SCREENTIME_TYPE_H

#include "info_type.h"

typedef struct : public Info{
    time_t timestamp_on;
    time_t timestamp_off;
    std::wstring user;
} ScreentimeInfo;

class ScreentimeCapture: public Capture {
public:
    explicit ScreentimeCapture();
    ~ScreentimeCapture() override = default;

    void sql_action(const Info*) override;
    static void terminate(Capture*,std::thread*);

private:
    void windowsStartup();
    ScreentimeInfo screentimeTracker;
    static bool exists;
    std::string screentimeTable;

    static ScreentimeCapture *self;
};
#endif //FLOBBEL_SCREENTIME_TYPE_H
