//
// Created by Chrystalkey on 12.07.2020.
//

#ifndef FLOBBEL_FLOB_EXCEPTIONS_H
#define FLOBBEL_FLOB_EXCEPTIONS_H

#include <exception>
#include <cstring>
#include "logging.h"

class flob_exception : public std::runtime_error {
public:
    flob_exception(const std::string &source, Log::InfoLevel lvl, const std::string &content);
};

class flob_info: public flob_exception{
public:
    flob_info(const std::string &src, const std::string &content);
};
class flob_warn: public flob_exception{
public:
    flob_warn(const std::string &src, const std::string &content);
};
class flob_recov:public flob_exception{
public:
    flob_recov(const std::string &src, const std::string &content);
};
class flob_panic:public flob_exception{
public:
    flob_panic(const std::string &src, const std::string &content);
};

class instance_exists_error : public flob_recov {
public:
    instance_exists_error(const std::string &src): flob_recov(src,"Only one instance permitted"){}
};

class sqlite_error : public flob_recov{
public:
    explicit sqlite_error(const std::string &src, const std::string &msg): flob_recov(src, msg){}
};


#endif //FLOBBEL_FLOB_EXCEPTIONS_H
