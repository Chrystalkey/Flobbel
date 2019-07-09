//
// Created by Chrystalkey on 01.07.2019.
//

#ifndef FLOBBEL_FLOBBELSAFE_H
#define FLOBBEL_FLOBBELSAFE_H

#include <fstream>
#include <queue>
#include <string>

#include <osrng.h>
#include "aes.h"
#include "modes.h"
#include "filters.h"

#include "global_functions.h"

class FlobbelSafe {
public:
    FlobbelSafe(std::wstring _safedir);
    ~FlobbelSafe();
    void finalize_files();
    void decrypt_copies();
    void encrypt_copies();
public:
    void add_key(KeypressInfo &);
    void add_prc(ProcessInfo &);
    void add_screentime(Screentime &);
private:
    std::queue<std::wstring> keyQueue;
    std::queue<std::wstring> prcQueue;
    std::wofstream keyOut, procOut, screenOut;
    std::wstring safedir;
private:
    //encryption stuff

};


#endif //FLOBBEL_FLOBBELSAFE_H
