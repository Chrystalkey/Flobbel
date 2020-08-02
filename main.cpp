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

#define ws(arg) from_string(#arg)
#define s(arg) (#arg)

// TODO: encrypt database
// TODO: add central capture structure
// TODO: add other captures
// TODO: fix concurrent writing access to db crashing program
// TODO: optionally enforce accumulation inserts

int main(int argc, char**argv) {
    if(argc > 1)
        if(!processArguments(argc, argv)) {
            Log::self->panic(ws(main(argc, argv)), L"Argument Processing failed");
            return 1;
        }
    try{
        initialize_flobbel();
    }catch(std::exception &e){
        Log::self->panic(ws(main), L"initialize_flobbel has failed. Terminating Program.");
        if(FlobbelSafe::self)delete FlobbelSafe::self;
        if(CaptureCollection::self)delete CaptureCollection::self;
        delete Log::self;
    }
    if(!SetConsoleCtrlHandler(control_handler, TRUE)) Log::self->recoverable(L"main",L"SetConsoleCtrlHandler failed");

    CaptureCollection::self->run();
    CaptureCollection::self->loop();
    CaptureCollection::self->terminate();

    delete FlobbelSafe::self;
    delete CaptureCollection::self;
    Log::self->info("main", "all cleaned up");
    delete Log::self;
    return 0;
}

bool processArguments(int argc, char **argv){
    std::vector<std::wstring> arguments(argc-1);
    for(int i = 1; i < argc; i++){
        arguments[i-1] =  from_string(std::string(argv[i]));
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
                Log::self->panic(L"processArguments", L"Probably no savedirectory specified. Please restart.");
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
        auto keypress = reinterpret_cast<const KeypressInfo&>(info);
        if(!(keypress.clickdown % 2)){
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
    Log::self->info(L"keySequence", L"Terminating Sequence complete");
    CaptureCollection::self->terminate();
}

void initialize_flobbel() {
    initMap();
    Log::init();
    FCS.mainthread_id = GetCurrentThreadId();
    //get uuid
    wchar_t appdataPath[MAX_PATH] = {0};
    if(SHGetFolderPathW(0, CSIDL_APPDATA, NULL, 0, appdataPath) != ERROR_SUCCESS){
        throw flob_panic("initialize_flobbel", "SHGetFolderPathW failed getting %appdata%");
    }
    FCS.savedirectory = std::wstring(appdataPath);
    FCS.db_path = FCS.savedirectory + L"/usr32.db";

    PathAppendW(appdataPath, L"usr32cat.log");
    bool writeHandleToFile = false;
    if(PathFileExistsW(appdataPath)){
        std::ifstream in(from_wstring(appdataPath));
        in >> FCS.handle;
        in.close();
    }else{
        FCS.handle = "REQUEST";
        writeHandleToFile = true;
    }

    if(writeHandleToFile){
        std::ofstream out(from_wstring(appdataPath), std::ios::trunc|std::ios::out);
        out << FCS.handle;
        out.close();
    }

    if(FCS.cleanup){
        DeleteFileW(FCS.db_path.c_str());
        DeleteFileW(appdataPath);
        Log::self->info(L"initialize_flobbel", L"finished cleanup");
        exit(0);
    }

    FlobbelSafe::init(FCS.savedirectory, safe_info_callback);
    CaptureCollection::init();

    Log::self->info(ws(initialize_flobbel), L"init finished");
}