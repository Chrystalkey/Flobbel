//
// Created by Chrystalkey on 01.07.2019.
//

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <windows.h>
#include <shlwapi.h>
#include "FlobbelSafe.h"

FlobbelSafe::FlobbelSafe(std::wstring &_safedir):
    safedir(_safedir){
    wchar_t date[11];
    auto n = now();
    int rc = 0;
    sqlite3_stmt *uniState = nullptr;
    wsprintfW(date,L"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    keyTable = L"KEYS_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";
    proTable = L"PROC_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";

    if(PathFileExistsW((safedir+L"flobsafe.db").c_str()))
        decrypt_file(safedir+L"flobsafe.db");
    rc = sqlite3_open16((safedir+L"flobsafe.db").c_str(),&dbcon);
    sqlErrCheck(rc,L"opening Database "+safedir+L"flobsafe.db",dbcon);
    //execStmt(dbcon,&uniState,L"CREATE DATABASE \"flobbel\";");
    execStmt(dbcon,&uniState,L"CREATE TABLE IF NOT EXISTS "+proTable+L" (pid INT(8),timestamp_on DATETIME, timestamp_off DATETIME, filename TEXT);");
    execStmt(dbcon,&uniState,L"CREATE TABLE IF NOT EXISTS "+keyTable+L" (updown CHAR(2),vkcode INT(8),scancode INT(8),description CHAR(5),timestamp DATETIME);");
    execStmt(dbcon,&uniState,L"CREATE TABLE IF NOT EXISTS screentime (cp_handle INT, timestamp_on DATETIME, timestamp_off DATETIME, duration INT);");
    sqlite3_finalize(uniState);
}
void FlobbelSafe::add_key(KeypressInfo &info){
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
void FlobbelSafe::add_prc(ProcessInfo &info){
    if(prcQueue.size()<13){
        prcQueue.push(L"("
                       +std::to_wstring(info.PID)+L","
                       +L"'"+info.timestamp_on+L"',"
                       +L"'"+info.timestamp_off+L"',"
                       +L"'"+info.filename+L"'),");
    }else{
        std::wstring vals;
        while(!prcQueue.empty()){
            vals += prcQueue.front();
            prcQueue.pop();
        }
        *(vals.end()-1) = L';';
        sqlite3_stmt *prcState = nullptr;
        execStmt(dbcon,&prcState,L"INSERT INTO "+proTable+L" (pid,timestamp_on,timestamp_off,filename) VALUES "+vals);
        sqlite3_finalize(prcState);
    }
}
void FlobbelSafe::add_screentime(Screentime &info){
    sqlite3_stmt *scrState = nullptr;
    execStmt(dbcon,&scrState,L"INSERT INTO screentime (cp_handle,timestamp_on,timestamp_off,duration) VALUES("
                              +std::to_wstring(info.ch)+L","
                              +L"'"+info.timestamp_on+L"',"
                              +L"'"+info.timestamp_off+L"',"
                              +std::to_wstring(info.duration)+L");"
    );
    sqlite3_finalize(scrState);
}
FlobbelSafe::~FlobbelSafe() {
    std::wstring vals;
    if (!prcQueue.empty()) {
        while (!prcQueue.empty()) {
            vals += prcQueue.front();
            prcQueue.pop();
        }
        *(vals.end() - 1) = L';';
        sqlite3_stmt *prcState = nullptr;
        execStmt(dbcon, &prcState,
                 L"INSERT INTO " + proTable + L" (pid,timestamp_on,timestamp_off,filename) VALUES " + vals);
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
    sqlite3_close(dbcon);
    //encrypt_n_copy(safedir+L"flobsafe.db");
    encrypt_file(safedir + L"flobsafe.db");
}
void FlobbelSafe::encrypt_file(const std::wstring &file) {
    std::ifstream in(converter.to_bytes(file), std::ios::binary|std::ios::ate);
    if(!in.is_open()){
        std::wcerr << "ERROR opening Infile(" << file << ") not opened\n";
        return;
    }
    std::streamsize size = in.tellg();
    in.seekg(0,std::ios::beg);

    buffer.resize(size, 0x00);
    if(in.read((char*)buffer.data(),size)){
        in.close();
        CryptoPP::SecByteBlock key(_key,32);
        CryptoPP::SecByteBlock iv(_iv,16);
        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption enc(key,key.size(),iv);
        enc.ProcessData(buffer.data(),buffer.data(),buffer.size());
        std::ofstream out(converter.to_bytes(file),std::ios::binary|std::ios::trunc);
        out.write((char*)buffer.data(),buffer.size());
        out.close();
    }else{
        std::wcout << "ERROR reading file(" << file+L"flobsafe.db" << ")\n";
    }
}
void FlobbelSafe::decrypt_file(const std::wstring &file) {
    std::ifstream in(converter.to_bytes(file), std::ios::binary|std::ios::ate);
    if(!in.is_open()){
        std::wcerr << "ERROR opening Infile(" << file << ") not opened\n";
        return;
    }
    std::streamsize size = in.tellg();
    in.seekg(0,std::ios::beg);

    buffer.resize(size, 0x00);
    if(in.read((char*)buffer.data(),size)){
        in.close();
        CryptoPP::SecByteBlock key(_key,32);
        CryptoPP::SecByteBlock iv(_iv,16);
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption dec(key,key.size(),iv);
        dec.ProcessData(buffer.data(),buffer.data(),buffer.size());
        std::ofstream out(converter.to_bytes(file),std::ios::binary|std::ios::trunc);
        out.write((char*)buffer.data(),buffer.size());
        out.close();
    }else{
        std::wcout << "ERROR reading file(" << file+L"flobsafe.db" << ")\n";
    }
}