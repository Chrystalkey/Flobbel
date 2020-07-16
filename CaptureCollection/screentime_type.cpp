//
// Created by Chrystalkey on 02.07.2020.
//

#include "screentime_type.h"
#include "../global_functions.h"
#include "../FlobbelSafe.h"
#include "capturecollection.h"

#include <windows.h>
#include <lm.h>
#include <chrono>

bool ScreentimeCapture::exists = false;

ScreentimeCapture::ScreentimeCapture() {
    if(exists)
        throw instance_exists_error("ScreentimeCallback");
    exists = true;
    sql_table = "CREATE TABLE IF NOT EXISTS screentime_type("
                "id INTEGER PRIMARY KEY,"
                "time_on TEXT,"
                "time_off TEXT,"
                "duration INTEGER);";
    infoType = FlobGlobal::InfoType::Screentime;
    FCS.safe->buildTable(sql_table);
    windowsStartup();
}
ScreentimeCapture::~ScreentimeCapture(){
    wmshutdownCallback();
}
void ScreentimeCapture::sql_action(const Info *info) {
    auto pack = (const ScreentimeInfo*) info;
    FCS.safe->insert_data(L"INSERT INTO screentime_type(time_on,time_off,duration) VALUES (?,?,?)",
                          {
                            new sql_str(pack->timestamp_on),
                            new sql_str(pack->timestamp_off),
                            new sql_int(pack->duration)
                          });
}
void ScreentimeCapture::windowsStartup() {
    NET_API_STATUS nStatus;
    LPUSER_INFO_2 ui2 = NULL;
    WCHAR uName[256];
    DWORD uNamen = sizeof(uName);

    GetUserNameW(uName,&uNamen);
    nStatus = NetUserGetInfo(NULL,uName,2,(LPBYTE*)&ui2);
    if(nStatus == ERROR_ACCESS_DENIED)
        std::cerr << "ERROR NetUserGetInfo failed becaus of access_denied error\n";
    time_t time = ui2->usri2_last_logon;
    NetApiBufferFree(ui2);
    wchar_t timestamp[20];
    struct tm *tm;
    tm = gmtime(&time);
    wsprintfW(timestamp,L"%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1,
              tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    screentimeTracker.ch = FCS.handle;
    screentimeTracker.infotype = infoType;
    screentimeTracker.timestamp_on = std::wstring(timestamp);
    screentimeTracker.on = time;
}

void ScreentimeCapture::wmshutdownCallback() {
    screentimeTracker.timestamp_off = timestamp();
    auto duration = std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(screentimeTracker.on);
    auto d = std::chrono::duration_cast<std::chrono::seconds>(duration);
    screentimeTracker.duration = d.count();
    FCS.safe->save(screentimeTracker);
}