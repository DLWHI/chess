#include "logger.h"
#include <ctime>
#include <cstdarg>
#include <stdio.h>
#include <stdexcept>

using namespace logger;

FILE* log_file = nullptr;

void logger::init(const char* log_file_name, bool append)
{
    if (log_file)
        return;
    if (append)
        if ((log_file = fopen(log_file_name, "a")))
            if (log(1, LOG_MESSAGE, "Initilized global logger bound to log file \"%s\" in append mode", log_file_name))
                return;
    if (!append)
        if ((log_file = fopen(log_file_name, "w")));
            if (log(1, LOG_MESSAGE, "Initilized global logger bound to log file \"%s\" in write mode", log_file_name))
                return;
    throw std::runtime_error("Failed to initialize logger file" + std::string(log_file_name));
}
int logger::log(int code, const char* level, const char* msg, ...)
{
    if (!log_file)
        if ((log_file = fopen("log.txt", "w")))
            log(0, LOG_MESSAGE, "Attempt to call logger without init, automatically opened \"log.txt\" for write");
    va_list args;
    va_start(args, msg);
    time_t now = time(0);
    fprintf(log_file, "%.24s", ctime(&now));
    fprintf(log_file, " [%s] ", level);
    vfprintf(log_file, msg, args);
    fprintf(log_file, "\n");
    fflush(log_file);
    va_end(args);
    return code;
}
void logger::close()
{
    fclose(log_file);
    log_file = nullptr;
}

Logger::Logger(const std::string& log_file, bool append)
{
    if (append)
        file.open(log_file, std::ios::openmode::_S_app);
    else
        file.open(log_file, std::ios::openmode::_S_out);
    if (file.is_open() && append)
        log(LOG_MESSAGE) << "Initilized logger instance bound to log file \"" << log_file << "\" in append mode" << std::endl;
    else if (file.is_open())
        log(LOG_MESSAGE) << "Initilized logger instance bound to log file \"" << log_file << "\" in write mode" << std::endl;
    
}
Logger::~Logger()
{
    file.close();
}

std::ofstream& Logger::log(const char* level)
{
    time_t now = time(0);
    file << std::string(ctime(&now)).substr(0, 24) << " [" << level << "] ";
    return file;
}
