//
// Created by Chrystalkey on 12.07.2020.
//

#include "logging.h"
#include "global_functions.h"

Log::Log(std::filesystem::path dir){
    logfile.open(dir);
    logfile << L"Flobbellog\n";
    logfile << L"["<<timestamp() << L"] [" << getOS() << "]\n";
    logfile << L"Active Captures:\n";
}
void Log::add_capture(const std::wstring &cap) {
    logfile << L"[" << cap << L"]\n";
}
void Log::finish_header(){
    headerfinished = true;
    logfile << buffer.str() << "\n\n";
}
void Log::write(const std::wstring &src, const std::wstring &msg, InfoLevel lvl) {
    if(headerfinished)
        logfile << L"[" << timestamp() << "] ["
        << (lvl==InfoLevel::Info?L"INFO":lvl==InfoLevel::Warning?L"WARNING":lvl==InfoLevel::Recoverable?L"Recoverable":L"Panic")
        << "] [" << src <<"] " << msg << std::endl;
    else
        buffer << L"[" << timestamp() << "] ["
        << (lvl==InfoLevel::Info?L"INFO":lvl==InfoLevel::Warning?L"WARNING":lvl==InfoLevel::Recoverable?L"Recoverable":L"Panic")
        << "] [" << src <<"] " << msg << std::endl;
}