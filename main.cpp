#include <iostream>
#include <deque>

#include "flobcallbackcollection.h"
#include "FlobbelSafe.h"

std::unordered_map<UINT, std::string> keys;
ComputerHandle globalHandle = -1;
std::set<std::string> blacklist;
std::string savedirectory;
std::string lookup_filepath;

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
    std::cout << info.filename << "|" << info.timestamp_on << " to " << info.timestamp_off << "\n";
    safe->add_prc(info);
}

void keypress(KeypressInfo info){
    std::string d(info.descr,4);
    if(info.updown %2 == 0) {
        std::cout << d << " " << std::hex << info.ch << "\n";
        keySequence(info.vkcode);
    }
    safe->add_key(info);
}

void screentime(Screentime info){
    std::cout << "Screentime: <on> " << info.timestamp_on << " <off> " << info.timestamp_off << " <duration> " << info.duration << " minutes\n";
    safe->add_screentime(info);
}

bool processArguments(int argc, char **argv){
    std::vector<std::string> arguments(argc-1);
    for(int i = 1; i < argc; i++){
        arguments[i-1] =  std::string(argv[i]);
    }
    for(auto it = arguments.begin(); it != arguments.end();it++){
        if(*it == "-bl" || *it == "--blacklist"){
            std::ifstream black;
            if((it+1) == arguments.end()){
                std::cerr << "No blacklist file specified. Please try again\n";
                return false;
            }
            try{
                black.open((*(it+1)));
            }catch(std::ios::failure fail){
                std::cerr << "Error Opening Blacklist File: " << fail.what() << "\n";
                return false;
            }catch(...){
                std::cerr << "Unknown error\n";
                return false;
            }
            std::string line;
            try{
                while(std::getline(black,line)){
                    if(!line.empty()) blacklist.insert(line);
                }
            }catch(...){
                std::cerr << "Error reading blacklist file. Please try again.\n";
                return false;
            }
        }
        if(*it == "-h"){
            std::cout << "Specify Blacklist File with -bl <filepath> or --blacklist <filepath>\n"
                      << "Show Help with -h\n"
                      << "Specify save directory with -sd <path> or --savedir <path>\n"
                      << "Specify computername Lookup File with -lu <filepath> or --lookup <filepath>\n";
        }
        if(*it == "-sd" || *it == "--savedir"){
            if((it+1) == arguments.end()){
                std::cerr << "Probably no savedirectory specified. Please restart.\n";
                return false;
            }
            savedirectory = *(it + 1);
        }
        if(*it == "-lu" || *it == "--lookup"){
            if((it+1) == arguments.end())
                std::cerr << "No lookup file specified. Please try again.\n";
            lookup_filepath = *(it+1);
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
    if(savedirectory.empty()) savedirectory = fucked_up_directory;
    safe = new FlobbelSafe(savedirectory);
    if(lookup_filepath.empty()) lookup_filepath = savedirectory+"lookupfile.data";
    globalHandle = getComputerHandle(lookup_filepath);
    callbackCollection = new FlobCallbackCollection(process,keypress, screentime);
    if(blacklist.empty()){
        blacklist.insert("svchost.exe");
        blacklist.insert("ctfmon.exe");
        blacklist.insert("RuntimeBroker.exe");
        blacklist.insert("conhost.exe");
        blacklist.insert("System");
        blacklist.insert("[System Process]");
        blacklist.insert("winlogon.exe");
    }
    callbackCollection->setProgramBlacklist(blacklist);
}