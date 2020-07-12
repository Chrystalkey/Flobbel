#ifndef FLOBBEL_GLOBAL_CONSTANTS_H
#define FLOBBEL_GLOBAL_CONSTANTS_H

#include <unordered_map>
#include <locale>
#include <codecvt>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include "flob_exceptions.h"

class Capture;

typedef unsigned char u_char;
typedef unsigned int UINT;

typedef std::string ComputerHandle;
typedef std::wstring Timestamp;

typedef std::thread* (*Runner)(Capture*);
typedef void (*Terminator)(Capture*, std::thread*);
typedef void (*MSGCallback)(Capture*, MSG &);

typedef struct{
    Capture* ptr = nullptr;
    Runner r = nullptr;
    Terminator t = nullptr;
    MSGCallback cb = nullptr;
    std::thread *th = nullptr;
} TripleThread;

typedef union {
    u_char* data[6] = {0};
    struct{
        u_char b0;
        u_char b1;
        u_char b2;
        u_char b3;
        u_char b4;
        u_char b5;
    };
} MAC;

class CaptureCollection;
class FlobbelSafe;
class Log;

class FlobConstants{
public:
    FlobConstants(){if(exists) std::cerr << "Please use only one instance of this\n";exists = true;}
    //ComputerHandle globalHandle = -1;
    std::string handle = "";
#ifdef __WIN32__
    std::wstring savedirectory;
    std::wstring db_path;
    std::unordered_map<UINT, std::wstring> keys;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t > > converter;
#else
    std::string savedirectory;
    std::string db_path;
    std::unordered_map<UINT, std::string> keys;
#endif
    bool cleanup = false;
    bool syncing = false;
    uint16_t ready_for_sync = 0; //0=ready, writing functions add up when writing, subtract when finished

    Log *log = nullptr;
    CaptureCollection *callbackCollection = nullptr;
    FlobbelSafe *safe = nullptr;
private:
    static bool exists;
};


extern FlobConstants FCS;

#endif //FLOBBEL_GLOBAL_CONSTANTS_H
