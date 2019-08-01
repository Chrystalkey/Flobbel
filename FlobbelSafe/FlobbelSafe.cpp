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

FlobbelSafe::FlobbelSafe(std::wstring &_safedir):mt(rd()),dist(60000,180000){
    wchar_t date[11];
    auto n = now();
    int rc = 0;
    tmrThread = new std::thread(&FlobbelSafe::timer,this);
    init_map();
    flobCS.db_path = _safedir+L"flobsafe.db";
    //find(); // TODO: add check whether sysLog.log exists, add fallback
    if(PathFileExistsW(flobCS.db_path.c_str())) {
        decrypt_file(flobCS.db_path);
    }
    sqlite3_stmt *uniState = nullptr;
    wsprintfW(date,L"%02d_%02d_%04d",n->tm_mday, n->tm_mon+1, n->tm_year+1900);
    keyTable = L"KEYS_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";
    proTable = L"PROC_"+std::wstring(date)+L"_"+std::wstring(computerHandleStr())+L"_CPACTIVITY";

    flobCS.ready_for_sync++;
    rc = sqlite3_open16(flobCS.db_path.c_str(),&dbcon);
    sqlErrCheck(rc,L"opening Database "+flobCS.db_path,dbcon);
    execStmt(dbcon,&uniState,L"CREATE TABLE IF NOT EXISTS "+proTable+L" (pid INT(8),timestamp_on DATETIME, timestamp_off DATETIME, filename TEXT);");
    execStmt(dbcon,&uniState,L"CREATE TABLE IF NOT EXISTS "+keyTable+L" (updown CHAR(2),vkcode INT(8),scancode INT(8),description CHAR(5),timestamp DATETIME);");
    execStmt(dbcon,&uniState,L"CREATE TABLE IF NOT EXISTS screentime (cp_handle INT, timestamp_on DATETIME, timestamp_off DATETIME, duration INT);");
    sqlite3_finalize(uniState);
    flobCS.ready_for_sync--;
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
void FlobbelSafe::add_screentime(const Screentime &info){
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
    end_timer = true;
    while(flobCS.syncing)Sleep(500);
    flobCS.ready_for_sync++;
    finalize_queues();
    sqlite3_close(dbcon);
    encrypt_file(flobCS.db_path);
    //hide();
}
void FlobbelSafe::encrypt_file(const std::wstring &file) {
    std::ifstream in(flobCS.converter.to_bytes(file), std::ios::binary|std::ios::ate);
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
        std::ofstream out(flobCS.converter.to_bytes(file),std::ios::binary|std::ios::trunc);
        out.write((char*)buffer.data(),buffer.size());
        out.close();
    }else{
        std::wcout << "ERROR reading file(" << file+L"flobsafe.db" << ")\n";
    }
}
void FlobbelSafe::decrypt_file(const std::wstring &file) {
    std::ifstream in(flobCS.converter.to_bytes(file), std::ios::binary|std::ios::ate);
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
        std::ofstream out(flobCS.converter.to_bytes(file),std::ios::binary|std::ios::trunc);
        out.write((char*)buffer.data(),buffer.size());
        out.close();
    }else{
        std::wcout << "ERROR reading file(" << file+L"flobsafe.db" << ")\n";
    }
}
void FlobbelSafe::save(const Info &info,Flob_constants::InfoType it) {
    while(flobCS.syncing)Sleep(500);
    flobCS.ready_for_sync++;
    switch(it){
        case Flob_constants::Process:
            add_prc(reinterpret_cast<const ProcessInfo&>(info));
            break;
        case Flob_constants::Keypress:
            add_key(reinterpret_cast<const KeypressInfo&>(info));
            break;
        case Flob_constants::Screentime:
            add_screentime(reinterpret_cast<const Screentime&>(info));
            break;
        default:
            std::cerr << "ERROR no other type possible @FlobbelSafe::save \n";
    }
    flobCS.ready_for_sync--;
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



void FlobbelSafe::timer() {
    uint32_t current_leap = 0;
    uint32_t current_rotation = 0;
    while(!end_timer){
        current_leap = dist(mt);
        current_rotation = 0;
        sync();
        while(current_rotation < current_leap){
            if(end_timer)break;
            current_rotation+=500;
            Sleep(500);
        }
    }
}
void FlobbelSafe::sync() {

}