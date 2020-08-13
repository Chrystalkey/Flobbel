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

ScreentimeCapture *ScreentimeCapture::self = nullptr;

ScreentimeCapture::ScreentimeCapture() {
    if(self)
        throw instance_exists_error("ScreentimeCapture::ScreentimeCapture");
    self = this;
    CaptureCollection::self->register_threading({this,nullptr,&ScreentimeCapture::terminate,nullptr,nullptr});
    screentimeTable = "Screentime_"+from_wstring(computerHandleStr());
    sql_table = "CREATE TABLE IF NOT EXISTS "+screentimeTable+"("
                "id INTEGER PRIMARY KEY,"
                "time_on INTEGER,"
                "time_off INTEGER,"
                "username TEXT);";
    infoType = FlobGlobal::InfoType::Screentime;
    FlobbelSafe::self->buildTable(sql_table);
    Log::self->info("ScreentimeCapture::ScreentimeCapture", "Table build verified: "+sql_table);
    windowsStartup();
}

void ScreentimeCapture::terminate(Capture *, std::thread *) {
    self->screentimeTracker.timestamp_off = ::time(0);
    self->sql_action(&(self->screentimeTracker));
}

void ScreentimeCapture::sql_action(const Info *info) {
    auto pack = (const ScreentimeInfo*) info;
    FlobbelSafe::self->insert_data(L"INSERT INTO "+from_string(screentimeTable)+L"(time_on, time_off, username) VALUES (?,?,?);",
                          {
                            new sql_int(pack->timestamp_on),
                            new sql_int(pack->timestamp_off),
                            new sql_str(pack->user)
                          });
}

void ScreentimeCapture::windowsStartup() {
    NET_API_STATUS nStatus;
    LPUSER_INFO_2 ui2 = NULL;
    WCHAR uName[256] = {0};
    DWORD uNamen = sizeof(uName);

    GetUserNameW(uName,&uNamen);
    nStatus = NetUserGetInfo(NULL,uName,2,(LPBYTE*)&ui2);

    if(nStatus == ERROR_ACCESS_DENIED) {
        Log::self->warning("ScreentimeCapture::windowsStartup", "NetUserGetInfo faile because of access_denied error");
    }

    screentimeTracker.ch = FCS::computer_handle;
    screentimeTracker.infotype = infoType;
    screentimeTracker.user = std::wstring(uName);
    screentimeTracker.timestamp_on = ::time(0);

    NetApiBufferFree(ui2);
}