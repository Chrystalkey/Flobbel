//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_GLOBAL_FUNCTIONS_H
#define FLOBBEL_GLOBAL_FUNCTIONS_H

#include "global_constants.h"
#include <time.h>

char *timestamp();
std::string map(uint32_t vkc);
void initMap();

ComputerHandle getComputerHandle(std::string lookupFile);
std::string computerName();
MAC mac();

std::string hexStr(u_char *data, size_t len);

void initialize_flobbel();

#endif //FLOBBEL_GLOBAL_FUNCTIONS_H
