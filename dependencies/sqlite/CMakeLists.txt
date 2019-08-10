project(sqlite)

add_library(sqlite STATIC shell.c sqlite3.c)