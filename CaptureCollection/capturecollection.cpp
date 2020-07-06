//
// Created by Chrystalkey on 19.06.2019.
//

#include "capturecollection.h"

CaptureCollection::CaptureCollection() {
    capture_classes.emplace(FlobGlobal::Keypress, std::make_unique<KeyboardCapture>());
    capture_classes.emplace(FlobGlobal::Process, std::make_unique<ProcessCapture>());
    capture_classes.emplace(FlobGlobal::Screentime, std::make_unique<ScreentimeCapture>());
}
