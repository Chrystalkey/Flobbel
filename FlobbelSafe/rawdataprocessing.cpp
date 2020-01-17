//
// Created by Chrystalkey on 04.01.2020.
//

#include "rawdataprocessing.h"

#include <iostream>
#include <fileapi.h>
#include <global_constants.h>

#include <cryptopp/files.h>
#include <cryptopp/gzip.h>
using namespace CryptoPP;

//#define TAG_SIZE 12

void RawDataProcessing::processToRubbish(const std::wstring &path) {
    compressFile(path);
    //encrypt_file(path);
}
void RawDataProcessing::processFromRubbish(const std::wstring &path) {
    //decrypt_file(path);
    decompressFile(path);
}
/*
void RawDataProcessing::encrypt_file(const std::wstring &file) {
AES::BLOCKSIZE;
    GCM< AES >::Encryption enc;
    enc.SetKeyWithIV(key, key.size(), iv, iv.size());
    auto test = (uint64_t)AES::BLOCKSIZE;
    FileSource(FCS.converter.to_bytes(file).c_str(),true,
            new AuthenticatedEncryptionFilter(enc,
                    new FileSink(FCS.converter.to_bytes(file+L".tmp").c_str()),false,TAG_SIZE
                                )
                    );

    if(!DeleteFileW(file.c_str())){
        std::cerr << "ERROR while deleting unencrypted DB File\n";
    }
    if(_wrename((file+L".tmp").c_str(), file.c_str())){
        std::cerr << "ERROR renaming encrypted db file\n";
    }
}
void RawDataProcessing::decrypt_file(const std::wstring &file) {
    GCM< AES >::Decryption dec;
    dec.SetKeyWithIV(key, key.size(), iv, iv.size());

    FileSource(FCS.converter.to_bytes(file).c_str(),true,
                        new AuthenticatedDecryptionFilter(dec,
                                                       new FileSink(FCS.converter.to_bytes(file+L".tmp").c_str()), false,TAG_SIZE
                        )
    );
    if(!DeleteFileW(file.c_str())){
        std::cerr << "ERROR while deleting encrypted DB File\n";
    }
    if(_wrename((file+L".tmp").c_str(), file.c_str())){
        std::cerr << "ERROR renaming decrypted db file\n";
    }
}
*/
void RawDataProcessing::compressFile(const std::wstring &path) {
    std::string file = FCS.converter.to_bytes(path);
    FileSource(file.c_str(),true,
            new Gzip(
                    new FileSink((file + ".tmp").c_str())
                    ));
    if(!DeleteFileA(file.c_str())){
        std::cerr << "ERROR while deleting uncompressed DB File\n";
    }
    if(rename((file+".tmp").c_str(), file.c_str())){
        std::cerr << "ERROR renaming compressed db file\n";
    }
}

void RawDataProcessing::decompressFile(const std::wstring &path) {
    std::string file = FCS.converter.to_bytes(path);
    FileSource(file.c_str(),true,
                      new Gunzip(
                              new FileSink((file + ".tmp").c_str())
                      ));
    if(!DeleteFileA(file.c_str())){
        std::cerr << "ERROR while deleting uncompressed DB File\n";
    }
    if(rename((file+".tmp").c_str(), file.c_str())){
        std::cerr << "ERROR renaming compressed db file\n";
    }
}
