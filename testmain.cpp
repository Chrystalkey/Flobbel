#include <regex>
#include <iostream>

//global function test
#include "global_functions.h"
#include "osrng.h"
#include "aes.h"
#include "modes.h"
#include "filters.h"


static const CryptoPP::byte _key[32] = {
        0xEF,0x46,0x94,0x5A,0x73,0xCE,0x2A,0xCF,
        0x97,0xD4,0x39,0x67,0x81,0x56,0x8C,0x06,
        0x0F,0x8A,0x6E,0x33,0xBC,0xFE,0xAB,0xBE,
        0x9A,0x22,0xA1,0xF7,0xA5,0x29,0x35,0xFD
};
static const CryptoPP::byte _iv[16] = {
        0x9E,0x9B,0x6A,0x8B,0xCF,0xB8,0x48,0x5F,
        0x6D,0xB8,0xD0,0x42,0x89,0x12,0xD3,0x2F
};

int main(int argc, char **argv){
    CryptoPP::SecByteBlock key(_key,32);
    CryptoPP::SecByteBlock iv(_iv, 16);
    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption enc(key,key.size(),iv);
    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption dec(key,key.size(),iv);

    CryptoPP::byte test[] = "Testtext.exe";
    std::cout << (char*)test << std::endl;

    enc.ProcessData(test,test,strlen((char*)test));
    std::cout << (char*)test << std::endl;
    dec.ProcessData(test,test,strlen((char*)test));
    std::cout << (char*)test << std::endl;
    return 0;
}