#include <string>
#include <deque>
#include <chrono>
#include <iostream>

enum LogLevel {
    LOG_NORMAL,
    LOG_WARNING,
    LOG_ERROR
};

struct LogEntry {
    std::time_t time;
    LogLevel level;
    std::string message;
};

class Log {
public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static Log* Instance() {
        static Log instance;
        return &instance;
    }

    void message(LogLevel level, const std::string& msg) {
        LogEntry entry{
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()),
                level,
                msg
        };

        log_entries.push_back(entry);

        if(log_entries.size() > 10) {
            log_entries.pop_front();
        }
    }

    void print() const {
        for(const auto& entry : log_entries) {
            std::tm* tm_info = std::localtime(&entry.time);
            char time_str[20];
            std::strftime(time_str, 20, "%Y-%m-%d %H:%M:%S", tm_info);

            std::cout << "[" << time_str << "] "
                      << levelToString(entry.level) << ": "
                      << entry.message << "\n";
        }
    }

private:
    Log() = default;
    ~Log() = default;

    std::deque<LogEntry> log_entries;

    std::string levelToString(LogLevel level) const {
        switch(level) {
            case LOG_NORMAL:  return "NORMAL ";
            case LOG_WARNING: return "WARNING";
            case LOG_ERROR:   return "ERROR  ";
        }
    }
};


int main() {
    Log* log = Log::Instance();

    log->message(LOG_NORMAL, "Program loaded");
    log->message(LOG_WARNING, "Low memory warning");
    log->message(LOG_ERROR, "Critical error detected!");

    log->print();

    return 0;
}