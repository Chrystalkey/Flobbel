//
// Created by Chrystalkey on 30.06.2019.
//

#ifndef FLOBBEL_GLOBAL_CONSTANTS_H
#define FLOBBEL_GLOBAL_CONSTANTS_H

#include <unordered_map>
#include <locale>
#include <codecvt>
#include <iostream>

#define fucked_up_directory L"D:/Temp/"

typedef unsigned char u_char;
typedef unsigned int UINT;

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
typedef std::string ComputerHandle;

typedef struct {
    ComputerHandle ch;
} Info;

typedef struct:public Info{
    std::wstring filename;
    std::wstring description;
    std::wstring timestamp_on;
    std::wstring timestamp_off;
    uint32_t PID = 0;
    bool done = false;
} ProcessInfo;

typedef struct:public Info{
    uint8_t updown = 0; // down == updown%2 == 0; up == updown%2 == 1
    uint32_t scancode = 0;
    uint32_t vkcode = 0;
    wchar_t descr[5] = {0};
    std::wstring timestamp;
} KeypressInfo;

typedef struct : public Info{
    std::wstring timestamp_on;
    std::wstring timestamp_off;
    time_t on;
    uint32_t duration;
} Screentime;

class FlobBase{
public:
    FlobBase();
    static void cleanup();
private:
    static FlobBase *emergency;
};

class FlobCallbackCollection;
class FlobbelSafe;
class FlobWS;
typedef struct FlobConstants{
    FlobConstants(){if(exists) std::cerr << "Please use only one instance of this\n";exists = true;}
    //ComputerHandle globalHandle = -1;
    std::string handle = "";
    std::wstring savedirectory;
    std::wstring db_path;
    std::unordered_map<UINT, std::wstring> keys;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t > > converter;
    bool syncing = false;
    uint16_t ready_for_sync = 0; //0=ready, writing functions add up when writing, subtract when finished
    enum InfoType{
        Process,
        Keypress,
        Screentime,
        MouseMove,
        MouseClick,
        WebConnect
    };
    //FlobBase base;
    FlobCallbackCollection *callbackCollection = nullptr;
    FlobbelSafe *safe = nullptr;
    FlobWS *flobWS = nullptr;
private:
    static bool exists;
} FlobConstants;

extern FlobConstants FCS;

class SynchroFailed{
public:
    SynchroFailed(std::string what);
    std::string what() const {return buffer;}
private:
    const std::string buffer;
};

#define INVALID_CP_HANDLE -1

#endif //FLOBBEL_GLOBAL_CONSTANTS_H
