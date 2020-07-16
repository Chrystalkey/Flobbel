//
// Created by Chrystalkey on 01.07.2019.
//

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include <cassert>
#include "FlobbelSafe.h"

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

#define intbind sqlite3_bind_int64(stmt, idx++, sqlite3_int64(arg));
#define strbind sqlite3_bind_text16(stmt,idx++,argtmp.c_str(),-1,nullptr);
void FlobbelSafe::insert_data(const std::wstring &sql, std::deque<sql_basetype*> bt) {
    if(sql.substr(0,11) == L"INSERT INTO"){
        sqlite3_stmt *statement = nullptr;
        wchar_t unused[256] = {0};
        prep_statement(dbcon, sql, &statement, unused);
        int32_t idx = 1;
        while(!bt.empty()){
            bind_arg(statement, idx++, bt.front());
            bt.pop_front();
        }
        int rc = sqlite3_step(statement);
        sqlErrCheck(rc, L"Stepping test insert. unused: "+ std::wstring(unused),dbcon);
    }else{
        throw(sqlite_error("not an \"insert\"-query @insert_data"));
    }
}

void FlobbelSafe::bind_arg(sqlite3_stmt *stmt, int32_t idx, const sql_basetype *bt) {
    if(bt->type() == SQLPassingType::Integer){
        sqlite3_bind_int64(stmt, idx, sqlite3_int64(*(sql_int*)bt));
         delete(sql_int*)bt;
    }else{
        std::wstring arg = *(sql_str*)bt;
        sqlite3_bind_text16(stmt,idx, arg.c_str(), sizeof(wchar_t)*(arg.length()), nullptr);
        delete(sql_str*)bt;
    }
}

void FlobbelSafe::buildTable(const std::string& sql){
    if(sql.substr(0,26) == "CREATE TABLE IF NOT EXISTS"){

    }else{
        throw sqlite_error("Not a table-creation string at buildTable");
    }
}

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