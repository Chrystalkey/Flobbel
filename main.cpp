#include <iostream>
#include <thread>
#include <deque>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <exception>

#include "FlobbelCallbackCollection/flobcallbackcollection.h"
#include <iptypes.h>
#include <iphlpapi.h>

std::unordered_map<UINT, std::string> keys;
ComputerHandle globalHandle = -1;
std::set<std::string> blacklist;

void process(ProcessInfo);
void keypress(KeypressInfo);

FlobCallbackCollection callbackCollection(process,keypress, nullptr);

int main() {
    initialize_flobbel();
    callbackCollection.run();
    return 0;
}

void process(ProcessInfo pInfo){
    std::cout << pInfo.filename << "|" << pInfo.timestamp_on << " to " << pInfo.timestamp_off << "\n";
}
void keypress(KeypressInfo info){
    std::string d(info.descr,4);
    if(info.updown %2 == 0) std::cout << d << " " << std::hex << info.ch << "\n";
}
void initialize_flobbel(){
    initMap();
    globalHandle = getComputerHandle(fucked_up_directory"lookupfile.data");
    blacklist.insert("svchost.exe");
    blacklist.insert("ctfmon.exe");
    callbackCollection.setProgramBlacklist(blacklist);
}
ComputerHandle getComputerHandle(std::string lookupFile){
    std::fstream lookup;

    try{
        lookup.open(lookupFile, std::ios::in|std::ios::out);
    }catch(std::ios_base::failure e){
        std::cout << "ERROR: " << e.what() << "\n";
    }
    if(!lookup.is_open()){
        std::cerr << "ERROR opening computer_handle lookup file\n";
        return 0;
    }
    std::string macadr = hexStr((u_char*)mac().data,6);
    std::string cpname = computerName();
    std::string line;
    size_t lineNumber = 0;
    bool append = false;

    while(lookup >> line){ // fixed line-size: at least 19 characters (1 character cpname)
        lineNumber++;
        if(line.find(macadr,0) != std::string::npos){
            if(line.find(cpname,0) != std::string::npos){
                line.replace(18,line.size()-19,cpname);
            }
            std::stringstream s;
            ComputerHandle ch;
            s << line.substr(0,4);
            s >> ch;
            return ch;
        }else if(line.find(cpname,0) != std::string::npos){
            if(line.find(macadr,0) != std::string::npos){
                line.replace(5,12,macadr);
            }
            std::stringstream s;
            ComputerHandle ch;
            s << line.substr(0,4);
            s >> ch;
            return ch;
        }else{
            append = true;
        }
    }
    if(!append && lineNumber == 0) append = true;
    if(append){
        lookup.seekp(lookup.end);
        lookup << std::setfill('0') << std::setw(4) << std::hex << lineNumber;
        lookup << ";" << macadr << ";" << cpname << "\n";
        lookup.flush();
        lookup.close();
    }
    return lineNumber;
}
std::string computerName(){
#define INFO_BUFFER_SIZE 32767
    TCHAR infoBuf[INFO_BUFFER_SIZE];
    DWORD bufCharCount = INFO_BUFFER_SIZE;

    if(!GetComputerName(infoBuf,&bufCharCount)){
        std::cerr << "ERROR While retrieving Computer Name\n";
        return "";
    }
    return std::string(infoBuf);
#undef INFO_BUFFER_SIZE
}
char *timestamp(){
    char *timestamp = (char *)malloc(sizeof(char) * 20);
    time_t ltime;
    ltime=time(nullptr);
    struct tm *tm;
    tm=localtime(&ltime);
    sprintf(timestamp,"%02d.%02d.%04d-%02d:%02d:%02d", tm->tm_mday, tm->tm_mon,
            tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return timestamp;
}
constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::string hexStr(u_char *data, size_t len){
    if(data == nullptr) return "";
    std::string s(len*2,' ');
    for(int i = 0; i < len; ++i){
        s[2*i]     = hexmap[(data[i]&0xF0)>>4];
        s[2*i+1]   = hexmap[(data[i]&0x0F)];
    }
    return s;
}
MAC mac(){
    std::vector<MAC> addresses;
    IP_ADAPTER_INFO adapterInfo[32];
    DWORD dwBufLen = sizeof(adapterInfo);
    DWORD dwStatus = GetAdaptersInfo(adapterInfo,&dwBufLen);
    PIP_ADAPTER_INFO pipAdapterInfo = adapterInfo;
    if(dwStatus != ERROR_SUCCESS){
        std::cerr << "ERROR getting adapterInfo\n";
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
std::string map(uint32_t vkc) {
    if (keys.count(vkc) == 0) {
        char chr = (char)MapVirtualKey(vkc, MAPVK_VK_TO_CHAR);
        if(chr==0) return "mist";
        return std::string("___") + chr;
    }
    return keys[vkc];
}
void initMap(){
    keys[VK_TAB]				= "_TAB";
    keys[VK_CAPITAL]			= "CAPS";
    keys[VK_F1]					= "__F1";
    keys[VK_F2]					= "__F2";
    keys[VK_F3]					= "__F3";
    keys[VK_F4]					= "__F4";
    keys[VK_F5]					= "__F5";
    keys[VK_F6]					= "__F6";
    keys[VK_F7]					= "__F7";
    keys[VK_F8]					= "__F8";
    keys[VK_F9]					= "__F9";
    keys[VK_F10]				= "_F10";
    keys[VK_F11]				= "_F11";
    keys[VK_F12]				= "_F12";
    keys[VK_F13]				= "_F13";
    keys[VK_F14]				= "_F14";
    keys[VK_F15]				= "_F15";
    keys[VK_F16]				= "_F16";
    keys[VK_F17]				= "_F17";
    keys[VK_F18]				= "_F18";
    keys[VK_F19]				= "_F19";
    keys[VK_F20]				= "_F20";
    keys[VK_F21]				= "_F21";
    keys[VK_F22]				= "_F22";
    keys[VK_F23]				= "_F23";
    keys[VK_F24]				= "_F24";
    keys[VK_BACK]				= "BACK";
    keys[VK_RETURN]				= "RETU";
    keys[VK_ESCAPE]				= "ESCA";
    keys[VK_LMENU]				= "LMEN";
    keys[VK_RMENU]				= "RMEN";
    keys[VK_LCONTROL]			= "LCTR";
    keys[VK_RCONTROL]			= "RCTR";
    keys[VK_RSHIFT]				= "RSHI";
    keys[VK_LSHIFT]				= "LSHI";
    keys[VK_LBUTTON]			= "LMBU";
    keys[VK_RBUTTON]			= "RMBU";
    keys[VK_CANCEL]				= "CANC";
    keys[VK_MBUTTON]			= "MBUT";
    keys[VK_XBUTTON1]			= "XBU1";
    keys[VK_XBUTTON2]			= "XBU2";
    keys[VK_CLEAR]				= "CLEA";
    keys[VK_PAUSE]				= "PAUS";
    keys[VK_FINAL]				= "FINA";
    keys[VK_CONVERT]			= "CONV";
    keys[VK_NONCONVERT]			= "NCVT";
    keys[VK_ACCEPT]				= "ACCE";
    keys[VK_MODECHANGE]			= "MODC";
    keys[VK_SPACE]				= "SPAC";
    keys[VK_PRIOR]				= "PRIO";
    keys[VK_NEXT]				= "NEXT";
    keys[VK_END]				= "_END";
    keys[VK_HOME]				= "HOME";
    keys[VK_LEFT]				= "LEFT";
    keys[VK_UP]					= "__UP";
    keys[VK_RIGHT]				= "RIGH";
    keys[VK_DOWN]				= "DOWN";
    keys[VK_SELECT]				= "SELE";
    keys[VK_PRINT]				= "PRNT";
    keys[VK_EXECUTE]			= "EXEC";
    keys[VK_SNAPSHOT]			= "SNAP";
    keys[VK_INSERT]				= "INSR";
    keys[VK_DELETE]				= "DELE";
    keys[VK_HELP]				= "HELP";
    keys[VK_LWIN]				= "LWIN";
    keys[VK_RWIN]				= "RWIN";
    keys[VK_APPS]				= "APPS";
    keys[VK_SLEEP]				= "SLEE";
    keys[VK_NUMPAD0]			= "NUM0";
    keys[VK_NUMPAD1]			= "NUM1";
    keys[VK_NUMPAD2]			= "NUM2";
    keys[VK_NUMPAD3]			= "NUM3";
    keys[VK_NUMPAD4]			= "NUM4";
    keys[VK_NUMPAD5]			= "NUM5";
    keys[VK_NUMPAD6]			= "NUM6";
    keys[VK_NUMPAD7]			= "NUM7";
    keys[VK_NUMPAD8]			= "NUM8";
    keys[VK_NUMPAD9]			= "NUM9";
    keys[VK_MULTIPLY]			= "MULT";
    keys[VK_ADD]				= "_ADD";
    keys[VK_SEPARATOR]			= "_SEP";
    keys[VK_SUBTRACT]			= "_SUB";
    keys[VK_DECIMAL]			= "DECI";
    keys[VK_DIVIDE]				= "_DIV";
    keys[VK_NUMLOCK]			= "NLCK";
    keys[VK_SCROLL]				= "SROL";
    keys[VK_BROWSER_BACK]		= "BBCK";
    keys[VK_BROWSER_FORWARD]	= "BFWD";
    keys[VK_BROWSER_REFRESH]	= "BREF";
    keys[VK_BROWSER_STOP]		= "BSTP";
    keys[VK_BROWSER_SEARCH]		= "BSEA";
    keys[VK_BROWSER_FAVORITES]	= "BFAV";
    keys[VK_BROWSER_HOME]		= "BHOM";
    keys[VK_VOLUME_MUTE]		= "VMUT";
    keys[VK_VOLUME_DOWN]		= "VDWN";
    keys[VK_VOLUME_UP]			= "_VUP";
    keys[VK_MEDIA_NEXT_TRACK]	= "MNXT";
    keys[VK_MEDIA_PREV_TRACK]	= "MPRV";
    keys[VK_MEDIA_STOP]			= "MSTP";
    keys[VK_MEDIA_PLAY_PAUSE]	= "MPLP";
    keys[VK_LAUNCH_APP1]		= "LAP1";
    keys[VK_LAUNCH_APP2]		= "LAP2";
    keys[VK_LAUNCH_MAIL]		= "LMAI";
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