//
// Created by Chrystalkey on 30.06.2019.
//

#ifndef FLOBBEL_GLOBAL_CONSTANTS_H
#define FLOBBEL_GLOBAL_CONSTANTS_H

#include <unordered_map>
#include <locale>
#include <codecvt>

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
typedef uint16_t ComputerHandle;

typedef struct{
    ComputerHandle ch;
    std::wstring filename;
    std::wstring description;
    std::wstring timestamp_on;
    std::wstring timestamp_off;
    uint32_t PID = 0;
    bool done = false;
} ProcessInfo;

typedef struct{
    ComputerHandle ch;
    uint8_t updown = 0; // down == updown%2 == 0; up == updown%2 == 1
    uint32_t scancode = 0;
    uint32_t vkcode = 0;
    wchar_t descr[5] = {0};
    std::wstring timestamp;
} KeypressInfo;

typedef struct{
    ComputerHandle ch;
    std::wstring timestamp_on;
    std::wstring timestamp_off;
    time_t on;
    std::wstring duration;
} Screentime;

extern std::unordered_map<UINT, std::wstring> keys;
extern ComputerHandle globalHandle;
extern std::wstring savedirectory;
extern std::wstring lookup_filepath;
extern std::wstring_convert<std::codecvt_utf8_utf16<wchar_t > > converter;


#endif //FLOBBEL_GLOBAL_CONSTANTS_H
