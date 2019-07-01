//
// Created by Chrystalkey on 30.06.2019.
//

#ifndef FLOBBEL_GLOBAL_CONSTANTS_H
#define FLOBBEL_GLOBAL_CONSTANTS_H

#include <unordered_map>

#define fucked_up_directory "D:/Temp/"

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

extern std::unordered_map<UINT, std::string> keys;
extern ComputerHandle globalHandle;


#endif //FLOBBEL_GLOBAL_CONSTANTS_H
