#include <iostream>
#include <deque>

#include <shlwapi.h>
#include <shlobj.h>
#include <memory>

#include "capturecollection.h"
#include "FlobbelSafe.h"
#include "capturetypes.h"
#include "logging.h"

bool FlobConstants::exists = false;
FlobConstants FCS;

void safe_info_callback(const Info&);
bool processArguments(int argc, char **argv);
void initialize_flobbel();
void keySequence(DWORD vkCode);

//TODO: put data safe-classes into capture-headers & use FlobSafe as a hub like CaptureCollection
//TODO: unify exceptions and logging

int main(int argc, char**argv) {
    if(argc > 1)
        if(!processArguments(argc, argv)) {
            FCS.log->panic(L"main", L"Argument Processing failed");
            return 1;
        }
    try{
        initialize_flobbel();
    }catch(std::exception){
        FCS.log->panic(L"main", L"initialize_flobbel failed. Terminating Program.");
        delete FCS.callbackCollection;
        delete FCS.safe;
        delete FCS.log;
    }
    if(!SetConsoleCtrlHandler(control_handler, TRUE)) FCS.log->recoverable(L"main",L"SetConsoleCtrlHandler failed");

    FCS.callbackCollection->run();
    FCS.callbackCollection->loop();

    delete FCS.callbackCollection;
    delete FCS.safe;
    delete FCS.log;
    return 0;
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
                FCS.log->panic(L"processArguments", L"Probably no savedirectory specified. Please restart.");
                return false;
            }
            FCS.savedirectory = *(it + 1);
        }
        if(*it == L"--cleanup"){
            FCS.cleanup = true;
        }
    }
    return true;
}

void safe_info_callback(const Info& info){
    if(info.infotype == FlobGlobal::Keypress){
        auto keypress = static_cast<const KeypressInfo&>(info);
        if(!(keypress.updown%2)){
            keySequence(keypress.vkcode);
        }
    }
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

    for(DWORD x : sequence)
        if(x != VK_ESCAPE)
            return;
    FCS.log->info(L"keySequence", L"Terminating Sequence complete");
    FCS.callbackCollection->terminate();
}

void initialize_flobbel() {
    initMap();
    FCS.log = new Log;
    //get uuid
    wchar_t appdataPath[MAX_PATH] = {0};
    if(SHGetFolderPathW(0,CSIDL_APPDATA, NULL, 0, appdataPath) != ERROR_SUCCESS){
        FCS.log->panic(L"initialize_flobbel", L"SHGetFolderPathW failed getting %appdata%");
        exit(1);
    }
    FCS.savedirectory = std::wstring(appdataPath);
    FCS.db_path = FCS.savedirectory + L"/usr32.db";

    PathAppendW(appdataPath, L"usr32cat.log");
    bool writeHandleToFile = false;
    if(PathFileExistsW(appdataPath)){
        std::ifstream in(FCS.converter.to_bytes(appdataPath));
        in >> FCS.handle;
        in.close();
    }else{
        FCS.handle = "REQUEST";
        writeHandleToFile = true;
    }

    if(writeHandleToFile){
        std::ofstream out(FCS.converter.to_bytes(appdataPath), std::ios::trunc|std::ios::out);
        out << FCS.handle;
        out.close();
    }

    if(FCS.cleanup){
        DeleteFileW(FCS.db_path.c_str());
        DeleteFileW(appdataPath);
        FCS.log->info(L"initialize_flobbel", L"finished cleanup");
        exit(0);
    }

    FCS.safe = new FlobbelSafe(FCS.savedirectory, safe_info_callback);

    FCS.callbackCollection = new CaptureCollection();
    try{
        FCS.callbackCollection->init();
    }catch(std::exception e){
        FCS.log->panic(L"initialize_flobbel", L"error while initializing Captures: "+FCS.converter.from_bytes(e.what()));
        throw std::exception(e);
    }
    FCS.log->info(L"initialize_flobbel", L"init finished");
}