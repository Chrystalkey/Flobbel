project(sqlite)

add_library(sqlite shell.c sqlite3.c)