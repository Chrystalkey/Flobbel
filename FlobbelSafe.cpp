//
// Created by Chrystalkey on 01.07.2019.
//

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include <cassert>
#include <mutex>
#include "FlobbelSafe.h"

FlobbelSafe* FlobbelSafe::self = nullptr;

void FlobbelSafe::init(std::filesystem::path _safedir, InfoCallback ic){
    if(self)throw instance_exists_error("FlobbelSafe::init");
    self = new FlobbelSafe(_safedir.wstring(), ic);
}

FlobbelSafe::FlobbelSafe(const std::wstring &_safedir, InfoCallback ic):mt(rd()),dist(60000,180000), ic(ic){

    if(PathFileExistsW(FCS.db_path.c_str())) {
        std::wifstream in(from_wstring(FCS.db_path));
        wchar_t buffer[7] = {0};
        in.read(buffer,6);
        in.close();
    }

    if(!createNInitDB(FCS.db_path.c_str(), &dbcon)){
        std::cerr << "ERROR creating normal DB connection\n";
    }
}
FlobbelSafe::~FlobbelSafe() {
    sqlite3_close(dbcon);
}

#define intbind sqlite3_bind_int64(stmt, idx++, sqlite3_int64(arg));
#define strbind sqlite3_bind_text16(stmt,idx++,argtmp.c_str(),-1,nullptr);

std::mutex db_write_mutex;
void FlobbelSafe::insert_data(const std::wstring &sql, std::deque<sql_basetype*> bt, int32_t module) {
    if(sql.substr(0,11) == L"INSERT INTO"){

        module = module?module:bt.size();
        wchar_t unused[256] = {0};
        int32_t idx = 0;

        db_write_mutex.lock();

        sqlite3_stmt *statement = nullptr;
        prep_statement(dbcon, sql, &statement, unused);
        while(!bt.empty()){
            //idx = ((idx)%module)+1;
            ++idx;
            bind_arg(statement, idx, bt.front());
            bt.pop_front();
        }
        int counter = 0;
        sql_try2:
        try{
            int rc = sqlite3_step(statement);
            sqlErrCheck(rc, L"Stepping insert. unused: "+ std::wstring(unused),dbcon);
        }catch(sqlite_error &sql_err) {
            if (++counter > 5) {
                throw flob_panic("void execStmt", std::string("giving up: ") + sql_err.what());
            } else {
                Sleep(100);
                goto sql_try2;
            }
        }
        sqlite3_finalize(statement);

        db_write_mutex.unlock();
    }else{
        throw(sqlite_error("FlobbelSafe::insert_data", "not an \"insert\"-query @insert_data"));
    }
}

void FlobbelSafe::bind_arg(sqlite3_stmt *stmt, int32_t idx, const sql_basetype *bt) {
    if(bt->type() == SQLPassingType::Integer){
        int64_t arg = *(sql_int*)bt;
        int rc = sqlite3_bind_int64(stmt, idx, sqlite3_int64(*(sql_int*)bt));
        sqlErrCheck(rc,L"binding argument: "+std::to_wstring(arg), dbcon);
        delete(sql_int*)bt;
    }else{
        std::wstring arg = *(sql_str*)bt;
        wchar_t *array = new wchar_t[arg.length()+1];
        wcscpy(array, arg.c_str());
        int rc = sqlite3_bind_text16(stmt, idx, array, sizeof(wchar_t)*(arg.length()),&::free);
        sqlErrCheck(rc,L"binding argument: "+arg, dbcon);
        delete(sql_str*)bt;
    }
}

void FlobbelSafe::buildTable(const std::string& sql){
    if(sql.substr(0,26) == "CREATE TABLE IF NOT EXISTS"){
        db_write_mutex.lock();
        sqlite3_stmt* stmt = nullptr;
        execStmt( dbcon, &stmt, from_string(sql));
        db_write_mutex.unlock();
    }else{
        throw sqlite_error("FlobbelSafe::buildTable","Not a table-creation string at buildTable");
    }
}

bool FlobbelSafe::createNInitDB(const std::wstring &path, sqlite3 **dbptr) {
    int rc = 0;
    sqlite3_stmt *uniState = nullptr;

    db_write_mutex.lock();
    rc = sqlite3_open16(path.c_str(), dbptr);

    sqlErrCheck(rc, L"opening Database " + path, *dbptr);
    execStmt(*dbptr, &uniState,L"CREATE TABLE IF NOT EXISTS testtable(id INTEGER PRIMARY KEY, blob TEXT, i BLOB);");
    sqlite3_finalize(uniState);
    db_write_mutex.unlock();

    return true;
}