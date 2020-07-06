#include <iostream>
#include <deque>

#include <shlwapi.h>
#include <shlobj.h>
#include <memory>

#include "capturecollection.h"
#include "FlobbelSafe.h"
#include "capturetypes.h"

bool FlobConstants::exists = false;
FlobConstants FCS;

void run();

void infoCallback(std::shared_ptr<Info> info_struct);

bool processArguments(int argc, char **argv);
void initialize_flobbel();
void keySequence(DWORD vkCode);

int main(int argc, char**argv) {
    if(argc > 1)
        if(!processArguments(argc, argv)) return 1;
    initialize_flobbel();
    if(!SetConsoleCtrlHandler(control_handler, TRUE)) std::cerr << "ERROR Doing SetConsoleCtrlHandler in main()\n";

    run();

    delete FCS.callbackCollection;
    delete FCS.safe;
    return 0;
}

/*
void process(ProcessInfo info){
    std::wcout << info.filename << L"|" << info.timestamp_on << L" to " << info.timestamp_off << L"\n";
    FCS.safe->save(info, FlobGlobal::Process);
}

void keypress(KeypressInfo info){
    std::wstring d(info.descr,4);
    if(info.updown %2 == 0) {
        std::wcout << d << L" " << FCS.converter.from_bytes(info.ch) << L"\n";
        keySequence(info.vkcode);
    }
    FCS.safe->save(info, FlobGlobal::Keypress);
}
*/

void run(){
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
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

    FCS.callbackCollection->terminate();
}

void initialize_flobbel() {
    initMap();

    //get uuid
    wchar_t appdataPath[MAX_PATH] = {0};
    if(SHGetFolderPathW(0,CSIDL_APPDATA, NULL, 0, appdataPath) != ERROR_SUCCESS){
        std::cerr << "ERROR retrieving appdata Path\n";
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
        exit(0);
    }

    FCS.safe = new FlobbelSafe(FCS.savedirectory, safe_info_callback);

    FCS.callbackCollection = new CaptureCollection();
}

//TODO: correctly destruct captureClasses