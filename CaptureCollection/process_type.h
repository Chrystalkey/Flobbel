//
// Created by Chrystalkey on 02.07.2020.
//

#ifndef FLOBBEL_PROCESS_TYPE_H
#define FLOBBEL_PROCESS_TYPE_H

#include "info_type.h"
#include <map>
#include <set>
#include <thread>

typedef struct: public Info{
#ifdef __WIN32__
    std::wstring filename;
    std::wstring description;
    std::wstring timestamp_on;
    std::wstring timestamp_off;
#else
    std::string filename;
    std::string description;
    std::string timestamp_on;
    std::string timestamp_off;
#endif
    bool execAtPrgmStart = false;
    bool execAtPrgmStop = false;
    uint32_t PID = 0;
    bool done = false;
} ProcessInfo;

class ProcessCapture: public Capture {
public:
    explicit ProcessCapture();
    ~ProcessCapture() override = default;

    static std::thread* run(Capture*);
    static void terminate(Capture *, std::thread*);
    void sql_action(const Info*) override;
private:
    std::map<uint32_t, ProcessInfo> *getProcessList();
    std::map<uint32_t, ProcessInfo> processList;
    std::set<std::wstring> blacklist = {L"svchost.exe", L"ctfmon.exe", L"RuntimeBroker.exe", L"conhost.exe",L"System", L"[System Process]", L"winlogon.exe",L"wininit.exe"};
    void updateProcessList();
    void detectChanges(std::map<uint32_t, ProcessInfo> *list);
    void finishProcessList();

    static ProcessCapture *self;
    bool _terminate = false;
    const std::wstring placeholder = L"platzhalterwobbel"; // platzhalterkonstante
    bool runsAtPrgmStart = false;
    static bool exists;
    std::string proTable;
};



#endif //FLOBBEL_PROCESS_TYPE_H
