//
// Created by Chrystalkey on 12.07.2020.
//

#ifndef FLOBBEL_LOGGING_H
#define FLOBBEL_LOGGING_H

#define CSSR const std::wstring&
#define CSR const std::string&

#include <fstream>
#include <filesystem>

class CaptureCollection;
class FlobSafe;

class Log{
public:
    enum InfoLevel{
        Info,
        Warning,
        Recoverable,
        Panic
    };
private:
    explicit Log(std::filesystem::path dir);
public:
    static void init(std::filesystem::path dir = L"floblog");
    static Log *self;
    void add_capture(CSSR cap);
    void finish_header();
    void write(CSSR src, CSSR msg, InfoLevel lvl);
    void write(CSR src, CSR msg, InfoLevel lvl);

    void info(CSSR src, CSSR msg){write(src,msg,InfoLevel::Info);}
    void info(CSR src, CSR msg){write(src,msg,InfoLevel::Info);}
    void warning(CSSR src, CSSR msg){write(src,msg,InfoLevel::Warning);}
    void warning(CSR src, CSR msg){write(src,msg,InfoLevel::Warning);}
    void recoverable(CSSR src, CSSR msg){write(src,msg,InfoLevel::Recoverable);}
    void recoverable(CSR src, CSR msg){write(src,msg,InfoLevel::Recoverable);}
    void panic(CSSR src, CSSR msg){write(src,msg,InfoLevel::Panic);}
    void panic(CSR src, CSR msg){write(src,msg,InfoLevel::Panic);}

    void datalog(const FlobSafe *, CSSR msg){info(L"FlobSafe",msg);}
    void registration(const CaptureCollection *c, CSSR msg){info(L"CaptureCollection::register_threading",msg);}
    void runner_go(const CaptureCollection *c, CSSR msg){info(L"CaptureCollection::run", msg);}
    void terminator_go(const CaptureCollection *c, CSSR msg){info(L"CaptureCollection::terminate", msg);}

private:
    std::wofstream logfile;
    std::wstringstream buffer;
    bool headerfinished = false;
};

#endif //FLOBBEL_LOGGING_H
