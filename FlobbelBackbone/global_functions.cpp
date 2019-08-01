//
// Created by Chrystalkey on 01.07.2019.
//
#include "global_functions.h"
#include <fstream>
#include <time.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <windows.h>
#include <vector>
#include <iptypes.h>
#include <iphlpapi.h>
#include <chrono>
#include "../dependencies/sqlite/sqlite3.h"

void sqlErrCheck(int rc, const std::wstring& additional, sqlite3 *dbcon){
    if(rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
        return;
    else{
        std::wcerr << "[ERROR] SQLITE failed " << additional << L" ERROR CODE: " << rc << L"\n";
        if(dbcon)
            sqlite3_close(dbcon);
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

ComputerHandle getComputerHandle(std::wstring lookupFile){
    std::string lookupFile_S = flobCS.converter.to_bytes(lookupFile);
    std::wfstream lookup;
    lookup.open(lookupFile_S, std::ios::out);
    lookup.close();
    lookup.open(lookupFile_S, std::ios::in|std::ios::out);
    std::wstringstream buffer;


    if(!lookup.is_open()){
        std::wcerr << L"ERROR opening computer_handle lookup file\n";
        return -1;
    }
    std::wstring macadr = hexStr((u_char*)mac().data,6);
    std::wstring cpname = computerName();
    std::wstring line;
    size_t lineNumber = 0;
    bool append = false;
    ComputerHandle retHandle;

    while(std::getline(lookup,line)){ // fixed line-size: at least 19 characters (1 character cpname)
        lineNumber++;
        if(line.find(macadr,0) != std::wstring::npos){
            if(line.find(cpname,0) != std::wstring::npos){
                line.replace(18,line.size()-19,cpname);
            }
            std::wstringstream s;
            ComputerHandle ch;
            s << line.substr(0,4);
            s >> retHandle;
        }else if(line.find(cpname,0) != std::wstring::npos){
            if(line.find(macadr,0) != std::wstring::npos){
                line.replace(5,12,macadr);
            }
            std::wstringstream s;
            s << line.substr(0,4);
            s >> retHandle;
        }else{
            append = true;
        }
        buffer << line;
    }
    if(!append && lineNumber == 0) append = true;
    if(append){
        buffer << std::setfill(L'0') << std::setw(4) << std::hex << lineNumber;
        buffer << L";" << macadr << L";" << cpname << "\n";
    }
    lookup.close();

    lookup.open(flobCS.converter.to_bytes(lookupFile), std::ios::out|std::ios::trunc);
    lookup << buffer.str();
    lookup.close();

    return lineNumber;
}
std::wstring computerName(){
#define INFO_BUFFER_SIZE 32767
    wchar_t infoBuf[INFO_BUFFER_SIZE];
    DWORD bufCharCount = INFO_BUFFER_SIZE;

    if(!GetComputerNameW(infoBuf,&bufCharCount)){
        std::wcerr << L"ERROR While retrieving Computer Name\n";
        return L"";
    }
    return std::wstring(infoBuf);
#undef INFO_BUFFER_SIZE
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
    return flobCS.converter.from_bytes(s);
}
std::wstring computerHandleStr(){
    static std::wstring handle;
    static wchar_t temp[6] = {0};
    if((int)temp[0] == 0){
        wsprintfW(temp,L"%05d",flobCS.globalHandle);
        handle = std::wstring(temp);
    }
    return handle;
}
MAC mac(){
    std::vector<MAC> addresses;
    IP_ADAPTER_INFO adapterInfo[32];
    DWORD dwBufLen = sizeof(adapterInfo);
    DWORD dwStatus = GetAdaptersInfo(adapterInfo,&dwBufLen);
    PIP_ADAPTER_INFO pipAdapterInfo = adapterInfo;
    if(dwStatus != ERROR_SUCCESS){
        std::wcerr << L"ERROR getting adapterInfo\n";
        return MAC();
    }
    while(pipAdapterInfo){
        if(pipAdapterInfo->Type == MIB_IF_TYPE_ETHERNET){
            MAC temp;
            memcpy(temp.data,pipAdapterInfo->Address,6);
            addresses.push_back(temp);
        }
        pipAdapterInfo = pipAdapterInfo->Next;
    }
    return addresses.front();
}
std::wstring map(uint32_t vkc) {
    if (flobCS.keys.count(vkc) == 0) {
        wchar_t chr = (wchar_t)MapVirtualKeyW(vkc, MAPVK_VK_TO_CHAR);
        if(chr==0) return L"mist";
        return std::wstring(L"___") + chr;
    }
    return flobCS.keys[vkc];
}
void initMap(){
    flobCS.keys[VK_TAB]				= L"_TAB";
    flobCS.keys[VK_CAPITAL]			= L"CAPS";
    flobCS.keys[VK_F1]					= L"__F1";
    flobCS.keys[VK_F2]					= L"__F2";
    flobCS.keys[VK_F3]					= L"__F3";
    flobCS.keys[VK_F4]					= L"__F4";
    flobCS.keys[VK_F5]					= L"__F5";
    flobCS.keys[VK_F6]					= L"__F6";
    flobCS.keys[VK_F7]					= L"__F7";
    flobCS.keys[VK_F8]					= L"__F8";
    flobCS.keys[VK_F9]					= L"__F9";
    flobCS.keys[VK_F10]				= L"_F10";
    flobCS.keys[VK_F11]				= L"_F11";
    flobCS.keys[VK_F12]				= L"_F12";
    flobCS.keys[VK_F13]				= L"_F13";
    flobCS.keys[VK_F14]				= L"_F14";
    flobCS.keys[VK_F15]				= L"_F15";
    flobCS.keys[VK_F16]				= L"_F16";
    flobCS.keys[VK_F17]				= L"_F17";
    flobCS.keys[VK_F18]				= L"_F18";
    flobCS.keys[VK_F19]				= L"_F19";
    flobCS.keys[VK_F20]				= L"_F20";
    flobCS.keys[VK_F21]				= L"_F21";
    flobCS.keys[VK_F22]				= L"_F22";
    flobCS.keys[VK_F23]				= L"_F23";
    flobCS.keys[VK_F24]				= L"_F24";
    flobCS.keys[VK_BACK]				= L"BACK";
    flobCS.keys[VK_RETURN]				= L"RETU";
    flobCS.keys[VK_ESCAPE]				= L"ESCA";
    flobCS.keys[VK_LMENU]				= L"LMEN";
    flobCS.keys[VK_RMENU]				= L"RMEN";
    flobCS.keys[VK_LCONTROL]			= L"LCTR";
    flobCS.keys[VK_RCONTROL]			= L"RCTR";
    flobCS.keys[VK_RSHIFT]				= L"RSHI";
    flobCS.keys[VK_LSHIFT]				= L"LSHI";
    flobCS.keys[VK_LBUTTON]			= L"LMBU";
    flobCS.keys[VK_RBUTTON]			= L"RMBU";
    flobCS.keys[VK_CANCEL]				= L"CANC";
    flobCS.keys[VK_MBUTTON]			= L"MBUT";
    flobCS.keys[VK_XBUTTON1]			= L"XBU1";
    flobCS.keys[VK_XBUTTON2]			= L"XBU2";
    flobCS.keys[VK_CLEAR]				= L"CLEA";
    flobCS.keys[VK_PAUSE]				= L"PAUS";
    flobCS.keys[VK_FINAL]				= L"FINA";
    flobCS.keys[VK_CONVERT]			= L"CONV";
    flobCS.keys[VK_NONCONVERT]			= L"NCVT";
    flobCS.keys[VK_ACCEPT]				= L"ACCE";
    flobCS.keys[VK_MODECHANGE]			= L"MODC";
    flobCS.keys[VK_SPACE]				= L"SPAC";
    flobCS.keys[VK_PRIOR]				= L"PRIO";
    flobCS.keys[VK_NEXT]				= L"NEXT";
    flobCS.keys[VK_END]				= L"_END";
    flobCS.keys[VK_HOME]				= L"HOME";
    flobCS.keys[VK_LEFT]				= L"LEFT";
    flobCS.keys[VK_UP]					= L"__UP";
    flobCS.keys[VK_RIGHT]				= L"RIGH";
    flobCS.keys[VK_DOWN]				= L"DOWN";
    flobCS.keys[VK_SELECT]				= L"SELE";
    flobCS.keys[VK_PRINT]				= L"PRNT";
    flobCS.keys[VK_EXECUTE]			= L"EXEC";
    flobCS.keys[VK_SNAPSHOT]			= L"SNAP";
    flobCS.keys[VK_INSERT]				= L"INSR";
    flobCS.keys[VK_DELETE]				= L"DELE";
    flobCS.keys[VK_HELP]				= L"HELP";
    flobCS.keys[VK_LWIN]				= L"LWIN";
    flobCS.keys[VK_RWIN]				= L"RWIN";
    flobCS.keys[VK_APPS]				= L"APPS";
    flobCS.keys[VK_SLEEP]				= L"SLEE";
    flobCS.keys[VK_NUMPAD0]			= L"NUM0";
    flobCS.keys[VK_NUMPAD1]			= L"NUM1";
    flobCS.keys[VK_NUMPAD2]			= L"NUM2";
    flobCS.keys[VK_NUMPAD3]			= L"NUM3";
    flobCS.keys[VK_NUMPAD4]			= L"NUM4";
    flobCS.keys[VK_NUMPAD5]			= L"NUM5";
    flobCS.keys[VK_NUMPAD6]			= L"NUM6";
    flobCS.keys[VK_NUMPAD7]			= L"NUM7";
    flobCS.keys[VK_NUMPAD8]			= L"NUM8";
    flobCS.keys[VK_NUMPAD9]			= L"NUM9";
    flobCS.keys[VK_MULTIPLY]			= L"MULT";
    flobCS.keys[VK_ADD]				= L"_ADD";
    flobCS.keys[VK_SEPARATOR]			= L"_SEP";
    flobCS.keys[VK_SUBTRACT]			= L"_SUB";
    flobCS.keys[VK_DECIMAL]			= L"DECI";
    flobCS.keys[VK_DIVIDE]				= L"_DIV";
    flobCS.keys[VK_NUMLOCK]			= L"NLCK";
    flobCS.keys[VK_SCROLL]				= L"SROL";
    flobCS.keys[VK_BROWSER_BACK]		= L"BBCK";
    flobCS.keys[VK_BROWSER_FORWARD]	= L"BFWD";
    flobCS.keys[VK_BROWSER_REFRESH]	= L"BREF";
    flobCS.keys[VK_BROWSER_STOP]		= L"BSTP";
    flobCS.keys[VK_BROWSER_SEARCH]		= L"BSEA";
    flobCS.keys[VK_BROWSER_FAVORITES]	= L"BFAV";
    flobCS.keys[VK_BROWSER_HOME]		= L"BHOM";
    flobCS.keys[VK_VOLUME_MUTE]		= L"VMUT";
    flobCS.keys[VK_VOLUME_DOWN]		= L"VDWN";
    flobCS.keys[VK_VOLUME_UP]			= L"_VUP";
    flobCS.keys[VK_MEDIA_NEXT_TRACK]	= L"MENX";
    flobCS.keys[VK_MEDIA_PREV_TRACK]	= L"MEPR";
    flobCS.keys[VK_MEDIA_STOP]			= L"MEST";
    flobCS.keys[VK_MEDIA_PLAY_PAUSE]	= L"MEPP";
    flobCS.keys[VK_LAUNCH_APP1]		= L"APP1";
    flobCS.keys[VK_LAUNCH_APP2]		= L"APP2";
    flobCS.keys[VK_LAUNCH_MAIL]		= L"LMAI";
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