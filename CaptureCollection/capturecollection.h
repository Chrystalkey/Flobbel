//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_CAPTURECOLLECTION_H
#define FLOBBEL_CAPTURECOLLECTION_H

#include "../global_functions.h"
#include <lm.h>
#include <map>
#include <memory>
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
    CaptureCollection();

    ~CaptureCollection() = default;
    void init();
    void run(){
        for(TripleThread &t:triplethreads){
            if(t.r and t.ptr) t.th = t.r(t.ptr);
        }
    }
    void terminate(){
        for(TripleThread &t:triplethreads){
            if(t.t and t.ptr and t.th) t.t(t.ptr,t.th);
        }
        PostQuitMessage(EXIT_SUCCESS);
    }

    void register_threading(TripleThread tt){triplethreads.push_back(tt);}

    void loop();
private:
    std::map<FlobGlobal::InfoType, std::unique_ptr<Capture>> capture_classes;
    std::vector<TripleThread> triplethreads;
};

#endif //FLOBBEL_CAPTURECOLLECTION_H
