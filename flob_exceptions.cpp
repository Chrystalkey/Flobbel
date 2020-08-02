//
// Created by Chrystalkey on 22.07.2020.
//

#include "flob_exceptions.h"
#include "global_functions.h"

flob_exception::flob_exception(const std::string &source, Log::InfoLevel lvl, const std::string &content) : std::runtime_error(content) {
    Log::self->write(from_string(source),from_string(content),lvl);
}

flob_info::flob_info(const std::string & src, const std::string & content):flob_exception(src,Log::Info, content) {}
flob_warn::flob_warn(const std::string & src, const std::string & content):flob_exception(src,Log::Warning, content) {}
flob_recov::flob_recov(const std::string & src, const std::string & content):flob_exception(src,Log::Recoverable, content) {}
flob_panic::flob_panic(const std::string & src, const std::string & content):flob_exception(src,Log::Panic, content) {}