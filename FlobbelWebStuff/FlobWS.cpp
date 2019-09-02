//
// Created by Chrystalkey on 01.08.2019.
//

#include <vector>
#include <set>
#include <sstream>
#include "FlobWS.h"
#include "global_functions.h"
#include "../dependencies/curl/include/curl/curl.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

FlobWS::FlobWS() {
    curl_global_init(CURL_GLOBAL_ALL);
}

namespace SyncSupport{
    std::string buffer;
    size_t receive(char *c_buf, size_t, size_t nmemb, void *){
        buffer.append(c_buf,nmemb);
        return nmemb*sizeof(char);
    }
}

/*
 * Ablauf:
 * Send request with os_version
 * receive all macs:  <mac1><mac2><mac3>...
 * determine whether mac exists in computer
 * send back yes|no or no|<mac>
 * receive computer handle, blacklist, rsd
 * */

std::set<std::wstring> FlobWS::sync_metadata() {
    CURL *down = curl_easy_init();
    std::string request;
    std::string host = "http://windefenddb.rf.gd/";
    // comment out if in production use
    host = "http://localhost/";
    // end of announcement
    //request += "get_metadata.php?mac="+mac+"&name="+curl_easy_escape(down,cpName.c_str(),cpName.length())+"&os="+FCS.converter.to_bytes(getOS());
    request += host + "verify_macs.php?os=" + FCS.converter.to_bytes(getOS());
    curl_easy_setopt(down, CURLOPT_URL, request.c_str());
    curl_easy_setopt(down, CURLOPT_WRITEFUNCTION, SyncSupport::receive);
    curl_easy_setopt(down, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(down, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(down, CURLOPT_VERBOSE,1L);
    CURLcode res = curl_easy_perform(down);
    if(res != CURLE_OK ) {
        std::cerr << "ERROR performing data sync: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(down);
        std::cerr << "Buffer Dump: " << SyncSupport::buffer << "\n";
        return std::set<std::wstring>();
    }
    std::cout << SyncSupport::buffer << "\n";
    //curl_easy_cleanup(down);

    bool is_registered = false;
    std::vector<std::string> *local_macs = mac();

    if(SyncSupport::buffer.empty()){
        std::cerr << "WARNING syncing buffer is empty\n";
    }else{
        for(int i = 0; i < SyncSupport::buffer.size()/12; i++){
            std::string tmp_mac = SyncSupport::buffer.substr(i*12,i*12+11);
            for(std::string &mc: *local_macs){
                if(mc == tmp_mac){
                    is_registered = true;
                    break;
                }
            }
            if(is_registered)
                break;
        }
        SyncSupport::buffer.clear();
    }

    if(is_registered){
        request = host+"register.php?state=yes&additional=no";
    }else{
        request = host+"register.php?state=no&additional="+local_macs->front();
    }

    curl_easy_setopt(down, CURLOPT_URL, request.c_str());
    res = curl_easy_perform(down);

    if(res != CURLE_OK ) {
        curl_easy_cleanup(down);
        std::cerr << "Buffer Dump: " << SyncSupport::buffer << "\n";
        throw SynchroFailed(std::string("ERROR: register.php cUrl return Code anormal: ")+ curl_easy_strerror(res));
    }

    std::cout << SyncSupport::buffer << " (is_registered)\n";

    if(SyncSupport::buffer.empty()){
        throw SynchroFailed("ERROR: register.php returned buffer is empty");
    }

    //later
    std::vector<std::string> resVec;
    std::istringstream iss(SyncSupport::buffer);
    std::string token;
    while(std::getline(iss,token,';')){
        resVec.push_back(token);
    }
    if(resVec.size() != 4 ){
        throw SynchroFailed("ERROR splitting received string; anormal number of entries\n");
    }
    FCS.savedirectory = FCS.converter.from_bytes(resVec[2]);
    FCS.db_path = FCS.savedirectory + FCS.converter.from_bytes(resVec.back());
    FCS.globalHandle = std::stoi(resVec.front());

    //and now: the blacklist
    std::string start = *(resVec.begin()+1);
    std::vector<std::string> out;
    std::set<std::wstring> bl;
    boost::split(out, start, boost::is_any_of("|"));
    for(std::string &prc: out){
        bl.insert(FCS.converter.from_bytes(prc));
    }
    for(std::wstring prc : bl){
        std::wcout << prc << L"|";
    }
    return bl;
}

namespace UploadSupport{
    size_t getContentLength(void *ptr, size_t size, size_t nmemb, void*stream){
        int r;
        long len = 0;
        r = sscanf((char*)ptr,"Content-Length: %ld\n",&len);
        if(r)
            *((long*)stream) = len;
        return size*nmemb;
    }
    size_t discard(void *ptr,size_t size, size_t nmemb, void *stream){
        (void)ptr;
        (void)stream;
        return size *nmemb;
    }
    size_t read(void *ptr, size_t size, size_t nmemb, void *stream){
        FILE *f = (FILE*)stream;
        size_t n;

        if(ferror(f)){
            return CURL_READFUNC_ABORT;
        }
        n = fread(ptr,size,nmemb,f)*size;
        return n;
    }
    size_t upload(CURL*curl, const std::wstring& remotePath,const std::wstring& localPath){
        int tries = 3;
        FILE *f;
        long uploaded_len = 0;
        CURLcode r = CURLE_GOT_NOTHING;
        int c;
        f = fopen(FCS.converter.to_bytes(localPath).c_str(), "rb");
        if(!f){
            perror(NULL);
            std::wcerr << "ERROR reading file " <<localPath << "\n";
            return 0;
        }
        curl_easy_setopt(curl, CURLOPT_UPLOAD,1L);
        curl_easy_setopt(curl, CURLOPT_URL, FCS.converter.to_bytes(localPath).c_str());
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, UploadSupport::getContentLength);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA,&uploaded_len);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read);
        curl_easy_setopt(curl, CURLOPT_READDATA,f);
        curl_easy_setopt(curl, CURLOPT_FTPPORT,"-");
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS,1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE,1L);

        for(c = 0; (r != CURLE_OK) && (c < tries);c++){
            if(c){
                curl_easy_setopt(curl,CURLOPT_NOBODY,1l);
                curl_easy_setopt(curl,CURLOPT_HEADER,1L);
                r = curl_easy_perform(curl);
                if(r != CURLE_OK){
                    continue;
                }
                curl_easy_setopt(curl,CURLOPT_NOBODY,0L);
                curl_easy_setopt(curl,CURLOPT_HEADER,0L);
                fseek(f,uploaded_len,SEEK_SET);
                curl_easy_setopt(curl,CURLOPT_APPEND, 1L);
            }else{
                curl_easy_setopt(curl,CURLOPT_APPEND,0L);
            }
            r = curl_easy_perform(curl);
        }
        fclose(f);
        if(r == CURLE_OK){
            return 1;
        }else{
            std::wcerr << "ERROR performing upload of file: " << curl_easy_strerror(r) << " - said curl\n";
            return 0;
        }
    }
    std::wstring generateDBName(){return L"BLAH.db";} // -> hash aus db; register db_hash + cpHandle with script call, save in DB
}

void FlobWS::upload(const std::wstring &file) {
    CURL *up = curl_easy_init();
    if(!up){
        std::cerr << "ERROR initializing curl handle up\n";
    }
    //curl_easy_setopt(up,CURLOPT_URL,"ftp://ftpupload.net/.htdocs/cp_names.xml");
    //curl_easy_setopt(up,CURLOPT_USERNAME,"epiz_24242735");
    //curl_easy_setopt(up,CURLOPT_PASSWORD,  "gYfJ6TI1wa");

    if(UploadSupport::upload(up,L"ftps://epiz_24242735:gYfJ6TI1wa@ftpupload.net/.htdocs/db_dir/"+UploadSupport::generateDBName(),file)){
        std::wcerr << "ERROR uploading file\n";
    }

    curl_easy_cleanup(up);
}

FlobWS::~FlobWS() {
    curl_global_cleanup();
}