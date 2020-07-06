//
// Created by Chrystalkey on 02.07.2020.
//

#ifndef FLOBBEL_SCREENTIME_TYPE_H
#define FLOBBEL_SCREENTIME_TYPE_H

#include "info_type.h"

static const std::string screentime_sql = "CREATE TABLE IF NOT EXISTS screentime_type("
                                          "id BIGINT PRIMARY KEY,"
                                          "time_on BIGINT,"
                                          "time_off BIGINT,"
                                          "duration BIGINT);";

typedef struct : public Info{
#ifdef __WIN32__
    std::wstring timestamp_on;
    std::wstring timestamp_off;
#else
    std::string timestamp_on;
    std::string timestamp_off;
#endif
    time_t on;
    uint32_t duration;
} ScreentimeInfo;

class ScreentimeCapture: public Capture {
public:
    explicit ScreentimeCapture();
    ~ScreentimeCapture();
private:
    void wmshutdownCallback();
    void windowsStartup();
    ScreentimeInfo screentimeTracker;
    static bool exists;
};
#endif //FLOBBEL_SCREENTIME_TYPE_H
