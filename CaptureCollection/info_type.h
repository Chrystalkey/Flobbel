//
// Created by Chrystalkey on 02.07.2020.
//

#ifndef FLOBBEL_INFO_TYPE_H
#define FLOBBEL_INFO_TYPE_H

#include "../global_constants.h"

namespace FlobGlobal{
    enum InfoType{
        Process,
        Keypress,
        Screentime,
        WLANList,
        MouseMove,
        MouseClick,
        WebConnect
    };
}

typedef struct {
    ComputerHandle ch;
    FlobGlobal::InfoType infotype;
} Info;

class Capture {
public:
    Capture();
    Capture(const Capture&)= delete;

    FlobGlobal::InfoType get_infotype() const {return infoType;}
protected:
    FlobGlobal::InfoType infoType;
};


#endif //FLOBBEL_INFO_TYPE_H
