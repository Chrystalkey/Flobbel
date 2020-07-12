//
// Created by Chrystalkey on 01.07.2019.
//
#include "global_functions.h"
#include <time.h>
#include <iostream>
#include <string>

#ifdef __WIN32__
#include <windows.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <versionhelpers.h>
#endif

#include <vector>
#include <chrono>

#include "sqlite/sqlite3.h"
#include "FlobbelSafe.h"
#include "logging.h"

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
        FCS.log->recoverable(L"getOS", L"GetVersionEx failed");
    return version;
}
#else
#endif

void sqlErrCheck(int rc, const std::wstring& additional, sqlite3 *dbcon){
    if(rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
        return;
    else{
        if(dbcon)
            sqlite3_close(dbcon);
        FCS.log->panic(L"sqlErrCheck", L"SQLite failed doing "+additional + L" with errcode: " + std::to_wstring(rc));
        throw sqlite_error("SQLite failed doing " + FCS.converter.to_bytes(additional) + " with errcode: " + std::to_string(rc));
    }
}

void prep_statement(sqlite3* dbcon, const std::wstring &stmt, sqlite3_stmt** statement, wchar_t *unused){
    int rc = sqlite3_prepare16_v2(dbcon,(void*)stmt.c_str(),(stmt.size()+1)*sizeof(wchar_t), statement,(const void**)&unused);
    sqlErrCheck(rc,L"preparing "+stmt+L" unused: "+unused);
}
void execStmt(sqlite3*dbcon, sqlite3_stmt**statement, const std::wstring &stmt){
    wchar_t unused[256];
    prep_statement(dbcon,stmt,statement,unused);
    int rc = sqlite3_step(*statement);
    sqlErrCheck(rc,L"stepping stmt: " + stmt+L" Unused: " + unused);
}

tm *now(){
    time_t ltime;
    ltime=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *tm;
    tm=gmtime(&ltime);
    return tm;
}
std::wstring timestamp(){
    wchar_t timestamp[20];
    auto tm = now();
    wsprintfW(timestamp,L"%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1,
            tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    std::wstring retour(timestamp);
    return retour;
}

constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::wstring hexStr(u_char *data, size_t len){
    if(data == nullptr) return L"";
    std::string s(len*2,' ');
    for(int i = 0; i < len; ++i){
        s[2*i]     = hexmap[(data[i]&0xF0)>>4];
        s[2*i+1]   = hexmap[(data[i]&0x0F)];
    }
    return FCS.converter.from_bytes(s);
}
std::wstring computerHandleStr(){
        return FCS.converter.from_bytes(FCS.handle);
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
/*std::vector<std::string> split(std::string input, char delim){
    std::vector<std::string> result;
    if(input.size() == 0 || delim == '\0')
        return result;
    std::size_t start = 0;
    char t = -1;
    for(int i = 0; i < input.size(); i++){
        t = input.at(i);
        if(t == delim){
            result.push_back(input.substr(start,i-start));
            start = i+1;
        }
    }
    result.push_back(input.substr(start, input.size()-start));

    return result;
}
std::vector<std::wstring> wsplit(std::wstring input, wchar_t delim){
    std::vector<std::wstring> result;
    if(input.size() == 0 || delim == L'\0')
        return result;
    std::size_t start = 0;
    wchar_t t = -1;
    for(int i = 0; i < input.size(); i++){
        t = input.at(i);
        if(t == delim){
            result.push_back(input.substr(start,i-start));
            start = i+1;
        }
    }
    result.push_back(input.substr(start, input.size()-start));

    return result;
}
std::vector<std::string> *mac(){
    static std::vector<MAC> addresses;
    IP_ADAPTER_INFO adapterInfo[32];
    DWORD dwBufLen = sizeof(adapterInfo);
    DWORD dwStatus = GetAdaptersInfo(adapterInfo,&dwBufLen);
    PIP_ADAPTER_INFO pipAdapterInfo = adapterInfo;
    if(dwStatus != ERROR_SUCCESS){
        std::wcerr << L"ERROR getting adapterInfo\n";
        return nullptr;
    }
    while(pipAdapterInfo){
        if(pipAdapterInfo->Type == MIB_IF_TYPE_ETHERNET || pipAdapterInfo->Type == IF_TYPE_IEEE80211){
            MAC temp;
            memcpy(temp.data,pipAdapterInfo->Address,6);
            addresses.push_back(temp);
        }
        pipAdapterInfo = pipAdapterInfo->Next;
    }
    std::cout << "MAC_SIZE: " << addresses.size() << "\n";
    char mac_construction[13] = {0};
    static std::vector<std::string> string_orchestra(addresses.size());
    for(MAC &m:addresses){
        sprintf(mac_construction,"%02x%02x%02x%02x%02x%02x", m.b0, m.b1, m.b2,m.b3,m.b4,m.b5);
        string_orchestra.emplace_back(std::string(mac_construction));
    }
    return &string_orchestra;
}*/

std::wstring map(uint32_t vkc) {
    if (FCS.keys.count(vkc) == 0) {
        wchar_t chr = (wchar_t)MapVirtualKeyW(vkc, MAPVK_VK_TO_CHAR);
        if(chr==0) return L"mist";
        return std::wstring(L"___") + chr;
    }
    return FCS.keys[vkc];
}

BOOL control_handler(DWORD signal){
    if(signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT){
        fprintf(stderr, "CTRL+C TRIGGERED\n");
    }
    return true;
}

void initMap(){
    FCS.keys[VK_TAB]			    = L"_TAB";
    FCS.keys[VK_CAPITAL]			= L"CAPS";
    FCS.keys[VK_F1]					= L"__F1";
    FCS.keys[VK_F2]					= L"__F2";
    FCS.keys[VK_F3]					= L"__F3";
    FCS.keys[VK_F4]					= L"__F4";
    FCS.keys[VK_F5]					= L"__F5";
    FCS.keys[VK_F6]					= L"__F6";
    FCS.keys[VK_F7]					= L"__F7";
    FCS.keys[VK_F8]					= L"__F8";
    FCS.keys[VK_F9]					= L"__F9";
    FCS.keys[VK_F10]				= L"_F10";
    FCS.keys[VK_F11]			    = L"_F11";
    FCS.keys[VK_F12]				= L"_F12";
    FCS.keys[VK_F13] 				= L"_F13";
    FCS.keys[VK_F14] 				= L"_F14";
    FCS.keys[VK_F15] 				= L"_F15";
    FCS.keys[VK_F16] 				= L"_F16";
    FCS.keys[VK_F17] 				= L"_F17";
    FCS.keys[VK_F18] 				= L"_F18";
    FCS.keys[VK_F19] 				= L"_F19";
    FCS.keys[VK_F20] 				= L"_F20";
    FCS.keys[VK_F21] 				= L"_F21";
    FCS.keys[VK_F22] 				= L"_F22";
    FCS.keys[VK_F23] 				= L"_F23";
    FCS.keys[VK_F24] 				= L"_F24";
    FCS.keys[VK_BACK]				= L"BACK";
    FCS.keys[VK_RETURN]				= L"RETU";
    FCS.keys[VK_ESCAPE]				= L"ESCA";
    FCS.keys[VK_LMENU]				= L"LMEN";
    FCS.keys[VK_RMENU]				= L"RMEN";
    FCS.keys[VK_LCONTROL]			= L"LCTR";
    FCS.keys[VK_RCONTROL]			= L"RCTR";
    FCS.keys[VK_RSHIFT]				= L"RSHI";
    FCS.keys[VK_LSHIFT]				= L"LSHI";
    FCS.keys[VK_LBUTTON] 			= L"LMBU";
    FCS.keys[VK_RBUTTON]	    	= L"RMBU";
    FCS.keys[VK_CANCEL]				= L"CANC";
    FCS.keys[VK_MBUTTON]		    = L"MBUT";
    FCS.keys[VK_XBUTTON1]			= L"XBU1";
    FCS.keys[VK_XBUTTON2]			= L"XBU2";
    FCS.keys[VK_CLEAR]				= L"CLEA";
    FCS.keys[VK_PAUSE]				= L"PAUS";
    FCS.keys[VK_FINAL]				= L"FINA";
    FCS.keys[VK_CONVERT]			= L"CONV";
    FCS.keys[VK_NONCONVERT]			= L"NCVT";
    FCS.keys[VK_ACCEPT]				= L"ACCE";
    FCS.keys[VK_MODECHANGE]			= L"MODC";
    FCS.keys[VK_SPACE]				= L"SPAC";
    FCS.keys[VK_PRIOR]				= L"PRIO";
    FCS.keys[VK_NEXT]				= L"NEXT";
    FCS.keys[VK_END]				= L"_END";
    FCS.keys[VK_HOME]				= L"HOME";
    FCS.keys[VK_LEFT]				= L"LEFT";
    FCS.keys[VK_UP]					= L"__UP";
    FCS.keys[VK_RIGHT]				= L"RIGH";
    FCS.keys[VK_DOWN]				= L"DOWN";
    FCS.keys[VK_SELECT]				= L"SELE";
    FCS.keys[VK_PRINT]				= L"PRNT";
    FCS.keys[VK_EXECUTE]			= L"EXEC";
    FCS.keys[VK_SNAPSHOT]			= L"SNAP";
    FCS.keys[VK_INSERT]				= L"INSR";
    FCS.keys[VK_DELETE]				= L"DELE";
    FCS.keys[VK_HELP]				= L"HELP";
    FCS.keys[VK_LWIN]				= L"LWIN";
    FCS.keys[VK_RWIN]				= L"RWIN";
    FCS.keys[VK_APPS]				= L"APPS";
    FCS.keys[VK_SLEEP]				= L"SLEE";
    FCS.keys[VK_NUMPAD0] 			= L"NUM0";
    FCS.keys[VK_NUMPAD1] 			= L"NUM1";
    FCS.keys[VK_NUMPAD2] 			= L"NUM2";
    FCS.keys[VK_NUMPAD3] 			= L"NUM3";
    FCS.keys[VK_NUMPAD4] 			= L"NUM4";
    FCS.keys[VK_NUMPAD5] 			= L"NUM5";
    FCS.keys[VK_NUMPAD6] 			= L"NUM6";
    FCS.keys[VK_NUMPAD7] 			= L"NUM7";
    FCS.keys[VK_NUMPAD8] 			= L"NUM8";
    FCS.keys[VK_NUMPAD9] 			= L"NUM9";
    FCS.keys[VK_MULTIPLY]			= L"MULT";
    FCS.keys[VK_ADD]		    	= L"_ADD";
    FCS.keys[VK_SEPARATOR]			= L"_SEP";
    FCS.keys[VK_SUBTRACT]			= L"_SUB";
    FCS.keys[VK_DECIMAL]		    = L"DECI";
    FCS.keys[VK_DIVIDE]				= L"_DIV";
    FCS.keys[VK_NUMLOCK]			= L"NLCK";
    FCS.keys[VK_SCROLL]				= L"SROL";
    FCS.keys[VK_BROWSER_BACK]		= L"BBCK";
    FCS.keys[VK_BROWSER_FORWARD] 	= L"BFWD";
    FCS.keys[VK_BROWSER_REFRESH] 	= L"BREF";
    FCS.keys[VK_BROWSER_STOP]		= L"BSTP";
    FCS.keys[VK_BROWSER_SEARCH]		= L"BSEA";
    FCS.keys[VK_BROWSER_FAVORITES]	= L"BFAV";
    FCS.keys[VK_BROWSER_HOME]		= L"BHOM";
    FCS.keys[VK_VOLUME_MUTE]		= L"VMUT";
    FCS.keys[VK_VOLUME_DOWN]		= L"VDWN";
    FCS.keys[VK_VOLUME_UP]			= L"_VUP";
    FCS.keys[VK_MEDIA_NEXT_TRACK]	= L"MENX";
    FCS.keys[VK_MEDIA_PREV_TRACK]	= L"MEPR";
    FCS.keys[VK_MEDIA_STOP]			= L"MEST";
    FCS.keys[VK_MEDIA_PLAY_PAUSE]	= L"MEPP";
    FCS.keys[VK_LAUNCH_APP1] 		= L"APP1";
    FCS.keys[VK_LAUNCH_APP2] 		= L"APP2";
    FCS.keys[VK_LAUNCH_MAIL] 		= L"LMAI";
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