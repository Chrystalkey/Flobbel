//
// Created by Chrystalkey on 01.08.2019.
//

#include <curl/curl.h>
#include <vector>
#include <set>
#include <sstream>
#include <Lmcons.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "FlobWS.h"
#include "global_functions.h"

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

std::set<std::wstring> FlobWS::sync_metadata() {
    std::string request = "";
    std::string host = ""; //hostname
    // comment out if in production use
    host = "http://localhost/";
    // end of announcement
    wchar_t uname[UNLEN+1] = {0};
    DWORD uname_len = UNLEN+1;
    if(!GetUserNameW(uname,&uname_len)) {
        wcscpy(uname,L"SOMETHINGWENTWRONG");
    }
    request += host + "register.php?uuid="+FCS.handle+"&os=" + FCS.converter.to_bytes(getOS()) + "&uname=" + FCS.converter.to_bytes(uname);
    {
        CURL *down = curl_easy_init();
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
        //std::cout << SyncSupport::buffer << "\n";
        curl_easy_cleanup(down);
    }
    if(FCS.handle == "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"){
        if(SyncSupport::buffer.substr(32, 4) != "|000"){
            std::cerr << "ERROR: Something went wrong with the request script: "<< SyncSupport::buffer.substr(16, SyncSupport::buffer.length()-16) << "\n";
        }
        FCS.handle = std::string(SyncSupport::buffer.substr(0,16));
    }
    wchar_t appdataPath[MAX_PATH] = {0};
    if(SHGetFolderPathW(0,CSIDL_APPDATA, NULL, 0, appdataPath) != ERROR_SUCCESS){
        std::cerr << "ERROR retrieving appdata Path\n";
        exit(1);
    }

    //blacklist download
    request.clear();
    request += host + "blacklist.php?";
    SyncSupport::buffer.clear();
    {
        CURL *down = curl_easy_init();
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
        //std::cout << SyncSupport::buffer << "\n";
        curl_easy_cleanup(down);
    }

    //blacklist division
    std::vector<std::string> resVec = split(SyncSupport::buffer, '/');
    if(resVec.empty()){
        throw SynchroFailed("ERROR splitting received string; resVec not full\n");
    }
    std::string start = resVec.front();
    std::set<std::wstring> bl;
    for(std::string &prc: resVec){
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

bool FlobWS::upload(const std::wstring &file) { //TODO: fill with upload logic

    return false;
}

FlobWS::~FlobWS() {
    curl_global_cleanup();
}