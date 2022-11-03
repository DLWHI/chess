#ifndef LOGGER_H_
#define LOGGER_H_

#define LOG_MESSAGE "MESSAGE"
#define LOG_WARNING "WARNING"
#define LOG_ERROR "ERROR"

#include <string>
#include <fstream>

namespace logger
{
    void init(const char* log_file_name = "log.txt", bool append = false);
    int log(int code, const char* level, const char* msg, ...);
    void close();

    class Logger
    {
        public:
            Logger(const std::string& log_file, bool append = false);
            ~Logger();

            std::ofstream& log(const char* level);
        private:
            std::ofstream file;
    };
}
#endif
