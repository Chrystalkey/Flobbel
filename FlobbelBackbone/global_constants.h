//
// Created by Chrystalkey on 30.06.2019.
//

#ifndef FLOBBEL_GLOBAL_CONSTANTS_H
#define FLOBBEL_GLOBAL_CONSTANTS_H

#include <unordered_map>

#define fucked_up_directory "D:/Temp/"

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
    std::string filename;
    std::string description;
    std::string timestamp_on;
    std::string timestamp_off;
    uint32_t PID = 0;
    bool done = false;
} ProcessInfo;

typedef struct{
    ComputerHandle ch;
    uint8_t updown = 0; // down == updown%2 == 0; up == updown%2 == 1
    uint32_t scancode = 0;
    uint32_t vkcode = 0;
    char descr[5] = {0};
    std::string timestamp;
} KeypressInfo;

typedef struct{
    ComputerHandle ch;
    std::string timestamp_on;
    std::string timestamp_off;
    time_t on;
    std::string duration;
} Screentime;

extern std::unordered_map<UINT, std::string> keys;
extern ComputerHandle globalHandle;


#endif //FLOBBEL_GLOBAL_CONSTANTS_H
