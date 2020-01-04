//
// Created by Chrystalkey on 04.01.2020.
//

#ifndef FLOBBEL_RAWDATAPROCESSING_H
#define FLOBBEL_RAWDATAPROCESSING_H

#include <string>
#include <vector>

class RawDataProcessing {
public:
    static void processToRubbish(const std::wstring &path);
    static void processFromRubbish(const std::wstring &path);
private:
    static void encrypt_file(const std::wstring &file);
    static void decrypt_file(const std::wstring &file);
private:
    //use something to compress the file
    static void compressFile(const std::wstring& path);
    static void decompressFile(const std::wstring& path);
};


#endif //FLOBBEL_RAWDATAPROCESSING_H
