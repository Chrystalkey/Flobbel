//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_CAPTURECOLLECTION_H
#define FLOBBEL_CAPTURECOLLECTION_H

#include "../global_functions.h"
#include "capturetypes.h"
#include <lm.h>
#include <map>
#include <memory>

class CaptureCollection {
public:
    CaptureCollection();

    CaptureCollection(const std::map<FlobGlobal::InfoType, std::unique_ptr<Capture>> &captureClasses);

    ~CaptureCollection() = default;

    void run(){
        static_cast<ProcessCapture*>(capture_classes[FlobGlobal::Process].get())->run();
    }
    void terminate(){
        static_cast<ProcessCapture*>(capture_classes[FlobGlobal::Process].get())->terminate();
        PostQuitMessage(EXIT_SUCCESS);
    }

private:
    std::map<FlobGlobal::InfoType, std::unique_ptr<Capture>> capture_classes;
};

#endif //FLOBBEL_CAPTURECOLLECTION_H
