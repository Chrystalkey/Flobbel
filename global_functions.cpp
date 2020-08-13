//
// Created by Chrystalkey on 01.07.2019.
//
#include "global_functions.h"
#include <time.h>
#include <string>

#ifdef __WIN32__
#include <windows.h>
//#include <iptypes.h>
//#include <iphlpapi.h>
#include <versionhelpers.h>
#endif

//#include <vector>
#include <chrono>
#include <mutex>

#include "sqlite/sqlite3.h"
#include "FlobbelSafe.h"
//#include "logging.h"

#ifdef __WIN32__
std::wstring getOS(){
    std::wstring version = L"WIN";

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(GetVersionEx(&osvi)){
        version += std::to_wstring(osvi.dwMajorVersion)+L"-"+std::to_wstring(osvi.dwMinorVersion);
        if (IsWindowsServer()) version+=L"SRV";
        if(sizeof(void*) == 8) version += L"64";
        else version += L"32";
    } else
        Log::self->recoverable(L"getOS", L"GetVersionEx failed");
    return version;
}
#else
#endif

std::mutex sql_err_mtx;

void sqlErrCheck(int rc, const std::wstring& additional, sqlite3 *dbcon){
    if(rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
        return;
    else{
        std::string message = "SQLite failed " + from_wstring(additional) + " with errcode: " + std::to_string(rc) + " and SQL errmsg: "+ sqlite3_errmsg(dbcon);
        if(dbcon) {
            sql_err_mtx.lock();
            sqlite3_close(dbcon);
            sql_err_mtx.unlock();
        }
        throw sqlite_error("sqlErrCheck", message);
    }
}

void prep_statement(sqlite3* dbcon, const std::wstring &stmt, sqlite3_stmt** statement, wchar_t *unused){
    int rc = sqlite3_prepare16_v2(dbcon,(void*)stmt.c_str(),(stmt.size()+1)*sizeof(wchar_t), statement,(const void**)&unused);
    sqlErrCheck(rc,L"preparing "+stmt+L" unused: "+unused);

}
void execStmt(sqlite3*dbcon, sqlite3_stmt**statement, const std::wstring &stmt){
    wchar_t unused[256];
    for (int counter = 0; counter < 6; counter++){
        prep_statement(dbcon,stmt,statement,unused);
        try{
            int rc = sqlite3_step(*statement);
            sqlErrCheck(rc,L"stepping stmt: " + stmt+L" unused: " + unused);
        }catch(sqlite_error &sql_err){
            if(counter >=5){
                throw flob_panic("void execStmt", std::string("giving up: ") + sql_err.what());
            }else{
                Sleep(100);
                sqlite3_reset(*statement);
            }
    }
    }
}

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t > > converter;

std::string from_wstring(const std::wstring &ws){
    return converter.to_bytes(ws);
}

std::wstring from_string(const std::string &s){
    return converter.from_bytes(s);
}

std::tm *now(){
    std::time_t t = std::time(0);
    std::tm *nw = std::localtime(&t);
    return nw;
}
std::wstring timestamp(){
    wchar_t timestamp[20];
    auto tm = now();
    wsprintfW(timestamp,L"%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1,
            tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    std::wstring retour(timestamp);
    return retour;
}

static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::wstring hexStr(const u_char *data, size_t len){
    if(data == nullptr) return L"";
    std::string s(len*2,' ');
    for(size_t i = 0; i < len; ++i){
        s[2*i]     = hexmap[(data[i]&0xF0u)>>4u];
        s[2*i+1]   = hexmap[(data[i]&0x0Fu)];
    }
    return from_string(s);
}
std::wstring computerHandleStr(){
        return from_string(FCS::computer_handle);
}
std::string generateHandle(){
    static const char alphanum[] = {
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIHKLMNOPQRSTUVWXYZ0123456789"
    };
    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_int_distribution<std::mt19937_64::result_type > dist(0,61);
    std::string result;
    for(int i = 0; i < 64; i++){
        result += alphanum[dist(rng)];
    }
    return result;
}

std::wstring map(uint32_t vkc) {
    if (FCS::vkcodes.count(vkc) == 0) {
        wchar_t chr = (wchar_t)MapVirtualKeyW(vkc, MAPVK_VK_TO_CHAR);
        if(chr==0) return L"mist";
        return std::wstring(L"___") + chr;
    }
    return FCS::vkcodes[vkc];
}

BOOL control_handler(DWORD signal){
    if(signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT){
        fprintf(stderr, "CTRL+C TRIGGERED\n");
    }
    return true;
}

void initMap(){
    FCS::vkcodes.insert({
    {VK_TAB,L"_TAB"},
    {VK_CAPITAL,L"CAPS"},
    {VK_F1,L"__F1"},
    {VK_F2,L"__F2"},
    {VK_F3,L"__F3"},
    {VK_F4,L"__F4"},
    {VK_F5,L"__F5"},
    {VK_F6,L"__F6"},
    {VK_F7,L"__F7"},
    {VK_F8,L"__F8"},
    {VK_F9,L"__F9"},
    {VK_F10,L"_F10"},
    {VK_F11,L"_F11"},
    {VK_F12,L"_F12"},
    {VK_F13,L"_F13"},
    {VK_F14,L"_F14"},
    {VK_F15,L"_F15"},
    {VK_F16,L"_F16"},
    {VK_F17,L"_F17"},
    {VK_F18,L"_F18"},
    {VK_F19,L"_F19"},
    {VK_F20,L"_F20"},
    {VK_F21,L"_F21"},
    {VK_F22,L"_F22"},
    {VK_F23,L"_F23"},
    {VK_F24,L"_F24"},
    {VK_BACK,L"BACK"},
    {VK_RETURN,L"RETU"},
    {VK_ESCAPE,L"ESCA"},
    {VK_LMENU,L"LMEN"},
    {VK_RMENU,L"RMEN"},
    {VK_LCONTROL,L"LCTR"},
    {VK_RCONTROL,L"RCTR"},
    {VK_RSHIFT,L"RSHI"},
    {VK_LSHIFT,L"LSHI"},
    {VK_LBUTTON,L"LMBU"},
    {VK_RBUTTON,L"RMBU"},
    {VK_CANCEL,L"CANC"},
    {VK_MBUTTON,L"MBUT"},
    {VK_XBUTTON1,L"XBU1"},
    {VK_XBUTTON2,L"XBU2"},
    {VK_CLEAR,L"CLEA"},
    {VK_PAUSE,L"PAUS"},
    {VK_FINAL,L"FINA"},
    {VK_CONVERT,L"CONV"},
    {VK_NONCONVERT,L"NCVT"},
    {VK_ACCEPT,L"ACCE"},
    {VK_MODECHANGE,L"MODC"},
    {VK_SPACE,L"SPAC"},
    {VK_PRIOR,L"PRIO"},
    {VK_NEXT,L"NEXT"},
    {VK_END,L"_END"},
    {VK_HOME,L"HOME"},
    {VK_LEFT,L"LEFT"},
    {VK_UP,L"__UP"},
    {VK_RIGHT,L"RIGH"},
    {VK_DOWN,L"DOWN"},
    {VK_SELECT,L"SELE"},
    {VK_PRINT,L"PRNT"},
    {VK_EXECUTE,L"EXEC"},
    {VK_SNAPSHOT,L"SNAP"},
    {VK_INSERT,L"INSR"},
    {VK_DELETE,L"DELE"},
    {VK_HELP,L"HELP"},
    {VK_LWIN,L"LWIN"},
    {VK_RWIN,L"RWIN"},
    {VK_APPS,L"APPS"},
    {VK_SLEEP,L"SLEE"},
    {VK_NUMPAD0,L"NUM0"},
    {VK_NUMPAD1,L"NUM1"},
    {VK_NUMPAD2,L"NUM2"},
    {VK_NUMPAD3,L"NUM3"},
    {VK_NUMPAD4,L"NUM4"},
    {VK_NUMPAD5,L"NUM5"},
    {VK_NUMPAD6,L"NUM6"},
    {VK_NUMPAD7,L"NUM7"},
    {VK_NUMPAD8,L"NUM8"},
    {VK_NUMPAD9,L"NUM9"},
    {VK_MULTIPLY,L"MULT"},
    {VK_ADD,L"_ADD"},
    {VK_SEPARATOR,L"_SEP"},
    {VK_SUBTRACT,L"_SUB"},
    {VK_DECIMAL,L"DECI"},
    {VK_DIVIDE,L"_DIV"},
    {VK_NUMLOCK,L"NLCK"},
    {VK_SCROLL,L"SROL"},
    {VK_BROWSER_BACK,L"BBCK"},
    {VK_BROWSER_FORWARD,L"BFWD"},
    {VK_BROWSER_REFRESH,L"BREF"},
    {VK_BROWSER_STOP,L"BSTP"},
    {VK_BROWSER_SEARCH,L"BSEA"},
    {VK_BROWSER_FAVORITES,L"BFAV"},
    {VK_BROWSER_HOME,L"BHOM"},
    {VK_VOLUME_MUTE,L"VMUT"},
    {VK_VOLUME_DOWN,L"VDWN"},
    {VK_VOLUME_UP,L"_VUP"},
    {VK_MEDIA_NEXT_TRACK,L"MENX"},
    {VK_MEDIA_PREV_TRACK,L"MEPR"},
    {VK_MEDIA_STOP,L"MEST"},
    {VK_MEDIA_PLAY_PAUSE,L"MEPP"},
    {VK_LAUNCH_APP1,L"APP1"},
    {VK_LAUNCH_APP2,L"APP2"},
    {VK_LAUNCH_MAIL,L"LMAI"}});
    /*
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_OEM_8 0xDF
#define VK_OEM_AX 0xE1
#define VK_OEM_102 0xE2
#define VK_ICO_HELP 0xE3
#define VK_ICO_00 0xE4
#define VK_PROCESSKEY 0xE5
#define VK_ICO_CLEAR 0xE6
#define VK_PACKET 0xE7
#define VK_OEM_RESET 0xE9
#define VK_OEM_JUMP 0xEA
#define VK_OEM_PA1 0xEB
#define VK_OEM_PA2 0xEC
#define VK_OEM_PA3 0xED
#define VK_OEM_WSCTRL 0xEE
#define VK_OEM_CUSEL 0xEF
#define VK_OEM_ATTN 0xF0
#define VK_OEM_FINISH 0xF1
#define VK_OEM_COPY 0xF2
#define VK_OEM_AUTO 0xF3
#define VK_OEM_ENLW 0xF4
#define VK_OEM_BACKTAB 0xF5
#define VK_ATTN 0xF6
#define VK_CRSEL 0xF7
#define VK_EXSEL 0xF8
#define VK_EREOF 0xF9
#define VK_PLAY 0xFA
#define VK_ZOOM 0xFB
#define VK_NONAME 0xFC
#define VK_PA1 0xFD
#define VK_OEM_CLEAR 0xFE
*/
}