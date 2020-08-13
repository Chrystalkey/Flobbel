//
// Created by Chrystalkey on 01.07.2019.
//

#ifndef FLOBBEL_FLOBBELSAFE_H
#define FLOBBEL_FLOBBELSAFE_H

#include <fstream>
#include <queue>
#include <string>
#include <random>
#include <thread>
#include <queue>
#include <mutex>

#include "dependencies/sqlite/sqlite3.h"

#include "global_functions.h"
#include "capturetypes.h"
#include "Ritualkacke/sqlpassingtypes.h"

typedef void (*InfoCallback)(const Info&);

class FlobbelSafe {
public:
    ~FlobbelSafe();
    void buildTable(const std::string& sql);
    static void init(const std::filesystem::path &_safedir, InfoCallback ic);
    //void save(const Info& info);
    static FlobbelSafe *self;
    void insert_data(const std::wstring &sql, std::deque<sql_basetype*> bt);
private:

    explicit FlobbelSafe(const std::wstring &_safedir, InfoCallback ic);

    void bind_arg(sqlite3_stmt*stmt, int32_t idx, const sql_basetype *bt);

    InfoCallback ic;
private: // PROCESSING QUEUES
    void finalize_queues();
    std::queue<std::wstring> keyQueue;
    std::queue<std::wstring> prcQueue;

private: //DATABASE INTERNA
    sqlite3 *dbcon = nullptr;
    std::wstring keyTable, proTable;
    static bool createNInitDB(const std::wstring &path, sqlite3 **dbptr);
    static std::mutex db_write_mutex;

};

#endif //FLOBBEL_FLOBBELSAFE_H
