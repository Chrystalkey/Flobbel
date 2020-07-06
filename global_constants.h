#ifndef FLOBBEL_GLOBAL_CONSTANTS_H
#define FLOBBEL_GLOBAL_CONSTANTS_H

#include <unordered_map>
#include <locale>
#include <codecvt>
#include <iostream>
#include <string>

typedef unsigned char u_char;
typedef unsigned int UINT;

typedef std::string ComputerHandle;

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
class FlobWS;

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
    //FlobBase base;
    CaptureCollection *callbackCollection = nullptr;
    FlobbelSafe *safe = nullptr;
    FlobWS *flobWS = nullptr;
private:
    static bool exists;
};

extern FlobConstants FCS;

class synchro_failed_error: public std::runtime_error {
public:
    synchro_failed_error(std::string what): std::runtime_error("Synchro Failed ERROR: "+what){}
};

class instance_exists_error : public std::runtime_error{
public:
    instance_exists_error(std::string e):std::runtime_error("Only one instance permitted: "+e){}
};

#endif //FLOBBEL_GLOBAL_CONSTANTS_H
