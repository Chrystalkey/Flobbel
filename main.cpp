#include <iostream>
#include <deque>

#include <winsock2.h>
#include "flobcallbackcollection.h"
#include "FlobbelSafe.h"
#include "FlobWS.h"

bool FlobConstants::exists = false;
FlobConstants FCS;

void process(ProcessInfo);
void keypress(KeypressInfo);
void screentime(Screentime);

bool processArguments(int argc, char **argv);
void initialize_flobbel();
void keySequence(DWORD vkCode);

int main(int argc, char**argv) {
    if(argc > 1){
        if(!processArguments(argc, argv)){
            return 1;
        }
    }
    initialize_flobbel();
    if(!SetConsoleCtrlHandler(control_handler, TRUE)){
        std::cerr << "ERROR Doing SetConsoleCtrlHandler\n";
    }
    FCS.callbackCollection->run();
    delete FCS.callbackCollection;
    delete FCS.safe;
    return 0;
}

void process(ProcessInfo info){
    std::wcout << info.filename << L"|" << info.timestamp_on << L" to " << info.timestamp_off << L"\n";
    FCS.safe->save(info, FlobConstants::Process);
}

void keypress(KeypressInfo info){
    std::wstring d(info.descr,4);
    if(info.updown %2 == 0) {
        std::wcout << d << L" " << std::hex << info.ch << L"\n";
        keySequence(info.vkcode);
    }
    FCS.safe->save(info, FlobConstants::Keypress);
}

void screentime(Screentime info){
    std::wcout << L"Screentime: <on> " << info.timestamp_on << L" <off> " << info.timestamp_off << L" <duration> "<< std::dec << info.duration << L" seconds\n";
    FCS.safe->save(info, FlobConstants::Screentime);
}

bool processArguments(int argc, char **argv){
    std::vector<std::wstring> arguments(argc-1);
    for(int i = 1; i < argc; i++){
        arguments[i-1] =  FCS.converter.from_bytes(std::string(argv[i]));
    }
    for(auto it = arguments.begin(); it != arguments.end();it++){
        if(*it == L"-h"){
            std::wcout << L"Show Help with -h\n"
                      << L"Specify save directory with -sd <path> or --savedir <path>\n"
                      << L"Specify computername Lookup File with -lu <filepath> or --lookup <filepath>\n";
        }
        if(*it == L"-sd" || *it == L"--savedir"){
            if((it+1) == arguments.end()){
                std::wcerr << L"Probably no savedirectory specified. Please restart.\n";
                return false;
            }
            FCS.savedirectory = *(it + 1);
        }
    }
    return true;
}

void keySequence(DWORD vkCode){
    static std::deque<DWORD> sequence;
    if(sequence.size()<4){
        sequence.push_back(vkCode);
        return;
    }else if(sequence.size()==4){
        sequence.push_back(vkCode);
    }else{
        sequence.pop_front();
        sequence.push_back(vkCode);
    }
    for(DWORD &x : sequence){
        if(x != VK_ESCAPE){
            return;
        }
    }
    FCS.callbackCollection->terminate();
}

void initialize_flobbel() {
    initMap();
    FlobWS flobWS;
    std::set<std::wstring> bl;
    try{
        bl = flobWS.sync_metadata();
    }catch(SynchroFailed sf){
        std::cerr << sf.what();
        FCS.globalHandle = INVALID_CP_HANDLE;
        wchar_t buffer[1024];
        GetTempPathW(1024, buffer);
        FCS.savedirectory = buffer;
        FCS.db_path = FCS.savedirectory+L"flobsave.db";

        bl.insert(L"svchost.exe");
        bl.insert(L"ctfmon.exe");
        bl.insert(L"RuntimeBroker.exe");
        bl.insert(L"conhost.exe");
        bl.insert(L"System");
        bl.insert(L"[System Process]");
        bl.insert(L"winlogon.exe");
        bl.insert(L"wininit.exe");
    }

    FCS.safe = new FlobbelSafe(FCS.savedirectory);
    FCS.callbackCollection = new FlobCallbackCollection(process,keypress, screentime);
    FCS.callbackCollection->setProgramBlacklist(bl);
}