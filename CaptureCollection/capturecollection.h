//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_CAPTURECOLLECTION_H
#define FLOBBEL_CAPTURECOLLECTION_H

#include <lm.h>
#include <map>
#include <memory>
#include "../global_functions.h"
#include "capturetypes.h"

typedef std::thread* (*Runner)(Capture*);
typedef void (*Terminator)(Capture*, std::thread*);
typedef void (*MSGCallback)(Capture*, const MSG &);

typedef struct{
    Capture* ptr = nullptr;
    Runner r = nullptr;
    Terminator t = nullptr;

    MSGCallback cb = nullptr;
    std::thread *th = nullptr;
} TripleThread;

class CaptureCollection {
public:
    static CaptureCollection *self;
    ~CaptureCollection();

    static void init();
    void run(){
        for(TripleThread &t:triplethreads){
            if(t.r and t.ptr) t.th = t.r(t.ptr);
        }
        Log::self->runner_go(this, L"All captures' infocollection threads are running.");
    }
    void terminate(){
        for(TripleThread &t:triplethreads){
            if(t.t and t.ptr) t.t(t.ptr, t.th);
        }
        Log::self->terminator_go(this, L"All captures' infocollection threads joined and terminated.");
    }

    void register_threading(TripleThread tt){triplethreads.push_back(tt);}

    void loop(bool (*callback)(const MSG&));
private:
    explicit CaptureCollection();

    std::map<FlobGlobal::InfoType, std::shared_ptr<Capture>> capture_classes;
    std::vector<TripleThread> triplethreads;
    std::thread *exitWatchdogThread = nullptr;
};

#endif //FLOBBEL_CAPTURECOLLECTION_H
