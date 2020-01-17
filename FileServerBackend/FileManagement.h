//
// Created by crystalkey on 17.01.20.
//

#ifndef FLOBBEL_FILEMANAGEMENT_H
#define FLOBBEL_FILEMANAGEMENT_H

#include <string>
#include <vector>
#include <sqlite3.h>

class sqlite3;

typedef std::vector<std::string> StrVec;

typedef struct FileInfo{
    std::string path;
    std::string hash; // non-compressed-file
    size_t fileSize;
    std::string computerHandle;
} FileInfo;

// save new files and set them into the database
class FileManagement {
public:
    explicit FileManagement(const std::string &root_dir);
    static void add_file(const std::string &path, FileInfo finfo); //I get finfo from the network answer
public:
    static std::string hash(const std::string &path);
public:
    // finders for files
private: //helper functions
    static void compress(const std::string &path, FileInfo *);
    static FileInfo decompress(const std::string &path, std::string dstPath);
    static bool create_n_init_db(const std::string &path, sqlite3 **dbptr);

    static void sqlErrCheck(int,const std::string&, sqlite3*dbcon = db_con);
    static void prep_statement(sqlite3*, const std::string&, sqlite3_stmt **, const char * unused);
    static void execStmt(sqlite3* dbcon, sqlite3_stmt** statement, const std::string& stmt);
private:
    static std::string root, temporary;
    static sqlite3* db_con;
    static bool exists_already;
};

#endif //FLOBBEL_FILEMANAGEMENT_H
