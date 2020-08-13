#ifndef FLOBBEL_GLOBAL_CONSTANTS_H
#define FLOBBEL_GLOBAL_CONSTANTS_H

#include <unordered_map>
#include <string>
#include <windef.h>
#include "flob_exceptions.h"

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
class Log;

namespace FCS{
    extern std::string computer_handle;
    extern std::wstring savedirectory;
    extern std::wstring db_path;
    extern std::unordered_map<UINT, std::wstring> vkcodes;
    extern bool cleanup;
    extern DWORD mainthread_id;
}
#endif //FLOBBEL_GLOBAL_CONSTANTS_H
