//
// Created by Chrystalkey on 03.08.2020.
//

#include "metadatatype.h"

MetadataCapture* MetadataCapture::self = nullptr;
MetadataInfo *MetadataCapture::currentState = nullptr;

MetadataCapture::MetadataCapture() {
    if(self)
        throw instance_exists_error("MetadataCapture::MetadataCapture");
    self = this;
    sql_table = "CREATE TABLE IF NOT EXISTS central_metadata_"+FCS::computer_handle+""
                "(id INTEGER PRIMARY KEY, uname TEXT, os_build TEXT,"
                "cpu_arch TEXT, cpu_verion TEXT,"
                "ram_version TEXT, ram_size INTEGER);";
}
