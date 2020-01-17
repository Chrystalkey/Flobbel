//
// Created by crystalkey on 17.01.20.
//

#include "FileManagement.h"
#include <sqlite3.h>

#include <crypto++/files.h>
#include <crypto++/gzip.h>
#include <crypto++/sha.h>
#include <crypto++/hex.h>
#include <sys/stat.h>

using namespace CryptoPP;

bool FileManagement::exists_already = false;
std::string FileManagement::root = "";
std::string FileManagement::temporary = "";

FileManagement::FileManagement(const std::string &root_dir) {
    if(exists_already){
        std::cerr << "ERROR: Only one instance of FileManagement allowed\n";
        exit(1);
    }
    exists_already = true;
    root = root_dir;
    temporary = (root_dir+"/temp/");
}

void FileManagement::add_file(const std::string &path, FileInfo finfo) {
    compress(path, &finfo);
}

void FileManagement::compress(const std::string &path, FileInfo *finfo) {
    FileSource(path.c_str(),true,
            new Gzip(
                    new FileSink((root+"/"+finfo->hash).c_str()),9
                    ));
}

FileInfo FileManagement::decompress(const std::string &path, std::string dstPath) {
    FileSource(path.c_str(), true,
               new Gunzip(
                       new FileSink(dstPath.c_str())
               ));
    FileInfo info;
    struct stat statbuf;
    if(stat(dstPath.c_str(),&statbuf) == -1){
        //check error how i will learn lateron. if i remember
    }

    info.fileSize = statbuf.st_size;
    info.path = dstPath;
    info.hash = hash(dstPath);
    //info.computerHandle = ?
}

std::string FileManagement::hash(const std::string &path) {
    SHA256 hash;
    std::string digest;

    FileSource(path.c_str(),true,
            new HashFilter(hash,
                    new HexEncoder(
                            new StringSink(digest)
                            )
                    )
            );
    return digest;
}

bool FileManagement::create_n_init_db(const std::string &path, sqlite3 **dbptr) {
    int rc = 0;
    sqlite3_stmt *uniState = nullptr;

    rc = sqlite3_open(path.c_str(), dbptr);

    sqlErrCheck(rc, "opening Database " + path, *dbptr);
    execStmt(*dbptr, &uniState, "CREATE TABLE IF NOT EXISTS files("
                                "id BIGINT PRIMARY KEY AUTOINCREMENT NOT NULL, "
                                "hash VARCHAR(32), "
                                "filesize BIGINT NOT NULL, "
                                "compHash VARCHAR(32), "
                                "compFileSize BIGINT NOT NULL, "
                                "computerHandle TEXT, "
                                "saveTimestamp TEXT);");
    sqlite3_finalize(uniState);

    return true;
}

void FileManagement::sqlErrCheck(int rc, const std::string& additional, sqlite3 *dbcon){
    if(rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
        return;
    else{
        std::cerr << "[ERROR] SQLITE failed " << additional << " ERROR CODE: " << rc << "\n";
        if(dbcon)
            sqlite3_close(dbcon);
    }
}

void FileManagement::prep_statement(sqlite3* dbcon, const std::string &stmt, sqlite3_stmt** statement, const char *unused){
    int rc = sqlite3_prepare_v2(dbcon,stmt.c_str(),-1, statement, &unused);
    sqlErrCheck(rc, "preparing " + stmt + " unused: " + unused);
}

void FileManagement::execStmt(sqlite3*dbcon, sqlite3_stmt**statement, const std::string &stmt){
    char unused[256];
    prep_statement(dbcon,stmt,statement,unused);
    int rc = sqlite3_step(*statement);
    sqlErrCheck(rc,"stepping stmt: " + stmt + " Unused: " + unused);
}
