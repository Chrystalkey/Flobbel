//
// Created by Chrystalkey on 01.08.2019.
//

#ifndef FLOBBEL_FLOBWS_H
#define FLOBBEL_FLOBWS_H

#include <iostream>

class FlobWS {
public:
    FlobWS();
    ~FlobWS();
    void upload(const std::wstring& file); //via ftp, call php afterwards
    std::set<std::wstring> sync_metadata(const std::string &mac, const std::string &cpName); // computerID, blacklist, random save directory (request php script with mac+cpName, that reads out of mysql)
    // computerID;blacklist1|blacklist2|...;randomSaveDir;database_name
};


#endif //FLOBBEL_FLOBWS_H
