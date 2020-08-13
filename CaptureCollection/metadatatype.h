//
// Created by Chrystalkey on 03.08.2020.
//

#ifndef FLOBBEL_METADATATYPE_H
#define FLOBBEL_METADATATYPE_H

#include "info_type.h"

typedef struct{
    MAC mac;
    std::wstring ip_ver;
    std::wstring name;
    std::wstring descr;
    std::wstring type;

    std::vector<uint32_t> local_ips;
    std::vector<uint32_t> gateways;

    bool dhcp_on;
    uint32_t dhcp_ip;
}NWAdapterBundle;

typedef struct{
    std::wstring username;
    std::wstring os_build;

    std::wstring CPU_architecture;
    std::wstring CPU_Version;

    std::wstring ram_version;
    uint64_t ram_size;

    NWAdapterBundle nwadapters;
}MetadataInfo;

class MetadataCapture: public Capture {
public:
    static MetadataCapture *self;

    static void init();
private:
    MetadataCapture();

    static MetadataInfo *currentState;

    std::string sql_table;

};


#endif //FLOBBEL_METADATATYPE_H
