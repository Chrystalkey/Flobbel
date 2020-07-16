//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_GLOBAL_FUNCTIONS_H
#define FLOBBEL_GLOBAL_FUNCTIONS_H

#include "global_constants.h"
#include <ctime>
#include <vector>
#include <windef.h>
#include <exception>
#include <stdexcept>

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

#define repeat_i(x) for(int i = 0; i < x; i++)
#ifdef __WIN32__
std::wstring timestamp();
std::wstring map(uint32_t vkc);
#else
std::string timestamp();
std::string map(uint32_t vkc);
#endif
void initMap();

#ifdef __WIN32__
std::wstring computerHandleStr();
std::wstring hexStr(u_char *data, size_t len);
#else
std::string computerHandleStr();
std::string hexStr(u_char *data, size_t len);
#endif

std::vector<std::string> *mac();
tm *now();
std::string generateHandle();
std::vector<std::wstring> wsplit(std::wstring input, wchar_t delim = L';');
std::vector<std::string> split(std::string input, char delim = ';');

#ifdef __WIN32__
std::wstring getOS();

void sqlErrCheck(int rc, const std::wstring& additional, sqlite3 *dbcon = nullptr);
void prep_statement(sqlite3* dbcon, const std::wstring &stmt, sqlite3_stmt** statement, wchar_t *unused);
void execStmt(sqlite3*dbcon, sqlite3_stmt**statement, const std::wstring &stmt);
template <class T>
void variprint(T arg);
template<class T, class ...Ts>
void variprint(T arg, Ts... args);

BOOL control_handler(DWORD signal);
#else
std::string getOS();

void sqlErrCheck(int rc, const std::string& additional, sqlite3 *dbcon = nullptr);
void prep_statement(sqlite3* dbcon, const std::string &stmt, sqlite3_stmt** statement, char *unused);
void execStmt(sqlite3*dbcon, sqlite3_stmt**statement, const std::string &stmt);

#endif

#endif //FLOBBEL_GLOBAL_FUNCTIONS_H
