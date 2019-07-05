//
// Created by Chrystalkey on 01.07.2019.
//

#ifndef FLOBBEL_FLOBBELSAFE_H
#define FLOBBEL_FLOBBELSAFE_H

#include <fstream>
#include <queue>
#include <string>
#include "global_functions.h"

class FlobbelSafe {
public:
    FlobbelSafe(std::string _safedir);
    ~FlobbelSafe();
    void finalize_files();
public:
    void add_key(KeypressInfo &);
    void add_prc(ProcessInfo &);
    void add_screentime(Screentime &);
private:
    std::queue<std::string> keyQueue;
    std::queue<std::string> prcQueue;
    std::ofstream keyOut, procOut, screenOut;
    std::string safedir;
};


#endif //FLOBBEL_FLOBBELSAFE_H
