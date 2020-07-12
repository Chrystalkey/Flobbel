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
        MouseInfo,
        MouseClick,
        MouseScroll,
        MouseMove,
        WLANList,
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
    std::string get_sql_table() const{return sql_table;}
protected:
    FlobGlobal::InfoType infoType;
    std::string sql_table;
    std::string sql_insert;
};


#endif //FLOBBEL_INFO_TYPE_H
