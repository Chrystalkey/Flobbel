//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_GLOBAL_FUNCTIONS_H
#define FLOBBEL_GLOBAL_FUNCTIONS_H

#include "global_constants.h"
#include <time.h>

#define repeat_i(x) for(int i = 0; i < x; i++)

std::wstring timestamp();
std::wstring map(uint32_t vkc);
void initMap();

ComputerHandle getComputerHandle(std::wstring lookupFile);
std::wstring computerName();
std::wstring computerHandleStr();
std::wstring hexStr(u_char *data, size_t len);
MAC mac();
tm *now();

#endif //FLOBBEL_GLOBAL_FUNCTIONS_H
