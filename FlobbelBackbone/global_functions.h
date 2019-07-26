//
// Created by Chrystalkey on 19.06.2019.
//

#ifndef FLOBBEL_GLOBAL_FUNCTIONS_H
#define FLOBBEL_GLOBAL_FUNCTIONS_H

#include "global_constants.h"
#include <time.h>

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

#define repeat_i(x) for(int i = 0; i < x; i++)

std::wstring timestamp();
std::wstring map(uint32_t vkc);
void initMap();

ComputerHandle getComputerHandle(std::wstring lookupFile);
std::wstring computerName();
std::wstring computerHandleStr();
std::wstring hexStr(u_char *data, size_t len);
MAC mac();
tm *now();
void sqlErrCheck(int rc, const std::wstring& additional, sqlite3 *dbcon = nullptr);
void prep_statement(sqlite3* dbcon, const std::wstring &stmt, sqlite3_stmt** statement, wchar_t *unused);
void execStmt(sqlite3*dbcon, sqlite3_stmt**statement, const std::wstring &stmt);
void initRandomDirectories();

#endif //FLOBBEL_GLOBAL_FUNCTIONS_H
