//
// Created by Chrystalkey on 12.07.2020.
//

#ifndef FLOBBEL_FLOB_EXCEPTIONS_H
#define FLOBBEL_FLOB_EXCEPTIONS_H

#include <exception>

class instance_exists_error : public std::runtime_error{
public:
    instance_exists_error(std::string e):std::runtime_error("Only one instance permitted: "+e){}
};

class sqlite_error : public std::runtime_error{
public:
    sqlite_error(std::string e):std::runtime_error(e){}
};

#endif //FLOBBEL_FLOB_EXCEPTIONS_H
