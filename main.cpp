#include <iostream>
#include <deque>

#include "flobcallbackcollection.h"
#include "FlobbelSafe.h"



bool Flob_constants::exists = false;
Flob_constants flobCS;
std::set<std::wstring> blacklist;

void process(ProcessInfo);
void keypress(KeypressInfo);
void screentime(Screentime);

bool processArguments(int argc, char **argv);
void initialize_flobbel();
void keySequence(DWORD vkCode);

FlobCallbackCollection *callbackCollection = nullptr;
FlobbelSafe *safe;

int main(int argc, char**argv) {
    if(argc > 1){
        if(!processArguments(argc, argv)){
            return 1;
        }
    }
    initialize_flobbel();
    callbackCollection->run();
    delete callbackCollection;
    delete safe;
    return 0;
}

void process(ProcessInfo info){
    std::wcout << info.filename << L"|" << info.timestamp_on << L" to " << info.timestamp_off << L"\n";
    safe->save(info,Flob_constants::Process);
}

void keypress(KeypressInfo info){
    std::wstring d(info.descr,4);
    if(info.updown %2 == 0) {
        std::wcout << d << L" " << std::hex << info.ch << L"\n";
        keySequence(info.vkcode);
    }
    safe->save(info,Flob_constants::Keypress);
}

void screentime(Screentime info){
    std::wcout << L"Screentime: <on> " << info.timestamp_on << L" <off> " << info.timestamp_off << L" <duration> "<< std::dec << info.duration << L" seconds\n";
    safe->save(info,Flob_constants::Screentime);
}

bool processArguments(int argc, char **argv){
    std::vector<std::wstring> arguments(argc-1);
    for(int i = 1; i < argc; i++){
        arguments[i-1] =  flobCS.converter.from_bytes(std::string(argv[i]));
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
            flobCS.savedirectory = *(it + 1);
        }
        if(*it == L"-lu" || *it == L"--lookup"){
            if((it+1) == arguments.end())
                std::wcerr << L"No lookup file specified. Please try again.\n";
            flobCS.lookup_filepath = *(it+1);
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
    callbackCollection->terminate();
}

void initialize_flobbel() {
    initMap();
    if(flobCS.savedirectory.empty()) flobCS.savedirectory = fucked_up_directory;
    if(flobCS.lookup_filepath.empty()) flobCS.lookup_filepath = flobCS.savedirectory+L"lookupfile.data";
    flobCS.globalHandle = getComputerHandle(flobCS.lookup_filepath);
    safe = new FlobbelSafe(flobCS.savedirectory);
    callbackCollection = new FlobCallbackCollection(process,keypress, screentime);
    if(blacklist.empty()){
        blacklist.insert(L"svchost.exe");
        blacklist.insert(L"ctfmon.exe");
        blacklist.insert(L"RuntimeBroker.exe");
        blacklist.insert(L"conhost.exe");
        blacklist.insert(L"System");
        blacklist.insert(L"[System Process]");
        blacklist.insert(L"winlogon.exe");
    }
    callbackCollection->setProgramBlacklist(blacklist);
}