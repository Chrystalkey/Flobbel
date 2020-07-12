//
// Created by Chrystalkey on 01.07.2019.
//

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include "FlobbelSafe.h"
//#include "FlobWS.h"

FlobbelSafe::FlobbelSafe(std::wstring &_safedir, InfoCallback ic):mt(rd()),dist(60000,180000), ic(ic){
    wchar_t date[11];
    auto n = now();
    if(PathFileExistsW(FCS.db_path.c_str())) {
        std::wifstream in(FCS.converter.to_bytes(FCS.db_path));
        wchar_t buffer[7] = {0};
        in.read(buffer,6);
        in.close();
    }
    wsprintfW(date,L"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    keyTable = L"KEYS_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";
    proTable = L"PROC_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";

    FCS.ready_for_sync++;
    if(!createNInitDB(FCS.db_path.c_str(), &dbcon)){
        std::cerr << "ERROR creating normal DB connection\n";
    }
    FCS.ready_for_sync--;
}
FlobbelSafe::~FlobbelSafe() {
    end_timer = true;
    while(FCS.syncing)Sleep(500);
    FCS.ready_for_sync++;
    finalize_queues();
    sqlite3_close(dbcon);
}
void FlobbelSafe::finalize_queues() {
    std::wstring vals;
    if (!prcQueue.empty()) {
        while (!prcQueue.empty()) {
            vals += prcQueue.front();
            prcQueue.pop();
        }
        *(vals.end() - 1) = L';';
        sqlite3_stmt *prcState = nullptr;
        execStmt(dbcon, &prcState,
                 L"INSERT INTO " + proTable + L" (pid,timestamp_on,timestamp_off,filename,execAtPrgmStart,execAtPrgmStop) VALUES " + vals);
        sqlite3_finalize(prcState);
    }
    if (!keyQueue.empty()) {
        vals = L"";
        while (!keyQueue.empty()) {
            vals += keyQueue.front();
            keyQueue.pop();
        }
        *(vals.end() - 1) = L';';
        sqlite3_stmt *keyState = nullptr;
        execStmt(dbcon, &keyState,
                 L"INSERT INTO " + keyTable + L" (updown,vkcode,scancode,description,timestamp) VALUES" + vals);
        sqlite3_finalize(keyState);
    }
}

void FlobbelSafe::add_key(const KeypressInfo &info){
    if(keyQueue.size() < 13){
        keyQueue.push(L"("+std::wstring(info.updown?L"'U'":L"'D'")+L","
                      +std::to_wstring(info.vkcode)+L","
                      +std::to_wstring(info.scancode)+L","
                      +L"'"+info.descr+L"',"
                      +L"'"+info.timestamp+L"'),");
    }else{
        std::wstring vals;
        while(!keyQueue.empty()){
            vals += keyQueue.front();
            keyQueue.pop();
        }
        *(vals.end()-1) = L';';
        sqlite3_stmt *keyState = nullptr;
        execStmt(dbcon,&keyState,L"INSERT INTO "+keyTable+L" (updown,vkcode,scancode,description,timestamp) VALUES" + vals);
        sqlite3_finalize(keyState);
    }
}
void FlobbelSafe::add_prc(const ProcessInfo &info){
    if(prcQueue.size()<13){
        prcQueue.push(L"("
                       +std::to_wstring(info.PID)                  +L","
                       +L"'"+info.timestamp_on                     +L"',"
                       +L"'"+info.timestamp_off                    +L"',"
                       +L"'"+info.filename                         +L"',"
                       +std::to_wstring((int)info.execAtPrgmStart) +L","
                       +std::to_wstring((int)info.execAtPrgmStop)  +L"),");
    }else{
        std::wstring vals;
        while(!prcQueue.empty()){
            vals += prcQueue.front();
            prcQueue.pop();
        }
        *(vals.end()-1) = L';';
        sqlite3_stmt *prcState = nullptr;
        execStmt(dbcon,&prcState,L"INSERT INTO "+proTable+L" (pid,timestamp_on,timestamp_off,filename,execAtPrgmStart,execAtPrgmStop) VALUES "+vals);
        sqlite3_finalize(prcState);
    }
}
void FlobbelSafe::add_screentime(const ScreentimeInfo &info){
    sqlite3_stmt *scrState = nullptr;
    execStmt(dbcon, &scrState,L"INSERT INTO screentime (cp_handle,timestamp_on,timestamp_off,duration) VALUES('"
                              +FCS.converter.from_bytes(info.ch)+L"',"
                              +L"'"+info.timestamp_on+L"',"
                              +L"'"+info.timestamp_off+L"',"
                              +std::to_wstring(info.duration)+L");"
    );
    sqlite3_finalize(scrState);
}
void FlobbelSafe::add_mouseclick(const MouseClickInfo &info){}
void FlobbelSafe::add_mousescroll(const MouseScrollInfo &info){}
void FlobbelSafe::add_mousemove(const MouseMoveInfo &info){}

void FlobbelSafe::save(const Info &info) {
    while(FCS.syncing) Sleep(500);
    FCS.ready_for_sync++;
    ic(info);
    switch(info.infotype){
        case FlobGlobal::Process:
            add_prc(static_cast<const ProcessInfo&>(info));
            break;
        case FlobGlobal::Keypress:
            add_key(static_cast<const KeypressInfo&>(info));
            break;
        case FlobGlobal::Screentime:
            //std::wcout << L"Screentime: <on> " << info.timestamp_on << L" <off> " << info.timestamp_off << L" <duration> "<< std::dec << info.duration << L" seconds\n";
            add_screentime(reinterpret_cast<const ScreentimeInfo&>(info));
            break;
        default:
            std::cerr << "ERROR no other type possible @FlobbelSafe::save \n";
    }
    FCS.ready_for_sync--;
}
/*
void FlobbelSafe::sync_timer() {
    uint32_t current_leap = 0;
    uint32_t current_rotation = 0;
    while(!end_timer){
        current_leap = dist(mt);
        current_rotation = 0;
        while(current_rotation < current_leap){
            if(end_timer)break;
            current_rotation+=500;
            Sleep(500);
        }
        sync();
    }
}

void FlobbelSafe::sync() {
    while(FCS.ready_for_sync) Sleep(100);
    FCS.syncing = true; //lock
    //open temporary DB for transfer
    if(!PathFileExistsW((FCS.db_path+ L".tmp").c_str())){ // file not there
        if( !createNInitDB(FCS.db_path + L".tmp", &uploadDBCon)){
            std::cerr << "ERROR creating upload sqliteDB\n";
            FCS.syncing = false;
            return;
        }
        CopyFileW(FCS.db_path.c_str(), (FCS.db_path+L".tmp").c_str(),false);
    }else{ // file already exists
        if(!createNInitDB(FCS.db_path + L".tmp", &uploadDBCon)){
            std::cerr << "ERROR connecting to existing uploadSqliteDB\n";
            FCS.syncing = false;
            return;
        }

        //Transfer Data from static to tmp DB
        sqlite3_stmt *uniState = nullptr;
        execStmt(uploadDBCon, &uniState, L"ATTACH '"+FCS.db_path+L"' AS srcDB;");

        execStmt(uploadDBCon, &uniState, L"INSERT INTO " + keyTable + L" SELECT * FROM srcDB."+keyTable);
        execStmt(uploadDBCon, &uniState, L"INSERT INTO " + proTable + L" SELECT * FROM srcDB."+proTable);
        execStmt(uploadDBCon, &uniState, L"INSERT INTO screentime SELECT * FROM srcDB.screentime");
        sqlite3_finalize(uniState);

        sqlite3_close(dbcon);
    }

    //Delete old DB
    DeleteFileW(FCS.db_path.c_str());

    // re-initialize new static DB
    wchar_t date[11];
    auto n = now();
    wsprintfW(date,L"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    keyTable = L"KEYS_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";
    proTable = L"PROC_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";

    //upload Database
    if( !createNInitDB(FCS.db_path + L".tmp", &uploadDBCon)){
        std::cerr << "ERROR creating nextGen Default sqliteDB\n";
        FCS.syncing = false;
        return;
    }
    if(!FCS.flobWS->upload(FCS.db_path+L".tmp")){
        std::cerr << "ERROR: Uploading File\n";
    }else{
        DeleteFileW((FCS.db_path+L".tmp").c_str());
    }
    std::wcerr << "Syncing\n";
    FCS.syncing = false; // unlock
}
*/
bool FlobbelSafe::createNInitDB(const std::wstring &path, sqlite3 **dbptr) {
    int rc = 0;
    sqlite3_stmt *uniState = nullptr;

    rc = sqlite3_open16(path.c_str(), dbptr);

    sqlErrCheck(rc, L"opening Database " + path, *dbptr);
    execStmt(*dbptr,&uniState,L"CREATE TABLE IF NOT EXISTS "+proTable+L" (pid INT(8),timestamp_on DATETIME, timestamp_off DATETIME, filename TEXT, execAtPrgmStart INT(1), execAtPrgmStop INT(1));");
    execStmt(*dbptr,&uniState,L"CREATE TABLE IF NOT EXISTS "+keyTable+L" (updown CHAR(2),vkcode INT(8),scancode INT(8),description CHAR(5),timestamp DATETIME);");
    execStmt(*dbptr,&uniState,L"CREATE TABLE IF NOT EXISTS screentime (cp_handle VARCHAR(32), timestamp_on DATETIME, timestamp_off DATETIME, duration INT);");
    sqlite3_finalize(uniState);

    return true;
}