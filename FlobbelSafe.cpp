//
// Created by Chrystalkey on 01.07.2019.
//

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include <cassert>
#include "FlobbelSafe.h"

FlobbelSafe* FlobbelSafe::self = nullptr;
std::mutex FlobbelSafe::db_write_mutex = std::mutex();

void FlobbelSafe::init(const std::filesystem::path &_safedir, InfoCallback ic){
    if(self)throw instance_exists_error("FlobbelSafe::init");
    self = new FlobbelSafe(_safedir.wstring(), ic);
}

FlobbelSafe::FlobbelSafe(const std::wstring &_safedir, InfoCallback ic): ic(ic){
    if(PathFileExistsW(FCS::db_path.c_str())) {
        std::wifstream in(from_wstring(FCS::db_path));
        wchar_t buffer[7] = {0};
        in.read(buffer,6);
        in.close();
    }

    if(!createNInitDB(FCS::db_path, &dbcon)){
        throw sqlite_error("FlobbelSafe::FlobbelSafe", "Failed to create a normal DB Connection");
    }
}
FlobbelSafe::~FlobbelSafe() {
    sqlite3_close(dbcon);
}

#define intbind sqlite3_bind_int64(stmt, idx++, sqlite3_int64(arg));
#define strbind sqlite3_bind_text16(stmt,idx++,argtmp.c_str(),-1,nullptr);

void FlobbelSafe::insert_data(const std::wstring &sql, std::deque<sql_basetype*> bt) {
    if(sql.substr(0,11) == L"INSERT INTO"){
        wchar_t unused[256] = {0};

        db_write_mutex.lock();

        sqlite3_stmt *statement = nullptr;

        for(int counter = 0; counter < 6;counter++){
            prep_statement(dbcon, sql, &statement, unused);

            auto iter = bt.begin();
            for(int idx = 1; iter != bt.end(); idx++, iter++){
                bind_arg(statement, idx, *iter);
            }

            try{
                int rc = sqlite3_step(statement);
                sqlErrCheck(rc, L"Stepping insert. unused: "+ std::wstring(unused),dbcon);
                break;
            }catch(sqlite_error &sql_err) {
                if (counter >= 5) {
                    throw flob_panic("void execStmt", std::string("giving up: ") + sql_err.what());
                } else {
                    Sleep(100);
                    sqlite3_reset(statement);
                    continue;
                }
            }
        }
        sqlite3_finalize(statement);
        for(auto arg:bt){if(arg->type()==Integer)delete(sql_int*)arg;else delete (sql_str*)arg;}
        bt.clear();
        db_write_mutex.unlock();
    }else{
        throw(sqlite_error("FlobbelSafe::insert_data", "not a valid \"insert\"-statement"));
    }
}

void FlobbelSafe::bind_arg(sqlite3_stmt *stmt, int32_t idx, const sql_basetype *bt) {
    if(bt->type() == SQLPassingType::Integer){
        int64_t arg = *(sql_int*)bt;
        int rc = sqlite3_bind_int64(stmt, idx, sqlite3_int64(*(sql_int*)bt));
        sqlErrCheck(rc,L"binding argument: "+std::to_wstring(arg), dbcon);
    }else{
        std::wstring arg = *(sql_str*)bt;
        wchar_t *array = new wchar_t[arg.length()+1];
        wcscpy(array, arg.c_str());
        int rc = sqlite3_bind_text16(stmt, idx, array, sizeof(wchar_t)*(arg.length()),&::free);
        sqlErrCheck(rc,L"binding argument: "+arg, dbcon);
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
    db_write_mutex.lock();
    sqlite3_stmt *uniState = nullptr;

    rc = sqlite3_open16(path.c_str(), dbptr);

    sqlErrCheck(rc, L"opening Database " + path, *dbptr);

    sqlite3_finalize(uniState);
    db_write_mutex.unlock();

    return true;
}