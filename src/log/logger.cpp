#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <filesystem>

// Static member initialization
bool Logger::initialized = false;
LogLevel Logger::currentLevel = LogLevel::DEBUG;
std::unique_ptr<std::ofstream> Logger::logFile = nullptr;

Logger::Logger(LogLevel level, const std::string& logfile) {
    if (!initialized) {
        init(level, logfile);
    }
}

Logger::~Logger() {
    // Close log file if open
    if (logFile) {
        logFile->close();
    }
}

void Logger::init(LogLevel level, const std::string& logfile) {
    if (initialized) {
        return;
    }
    
    currentLevel = level;
    
    std::string logfilePath = logfile;
    if (logfilePath.empty()) {
        // Use default log file path: ~/.config/doowm/doowm.log
        const char* homeDir = std::getenv("HOME");
        if (homeDir) {
            try {
                // ファイルシステム操作を安全に行う
                std::filesystem::path dirPath = std::filesystem::path(homeDir) / ".config" / "doowm";
                
                if (!std::filesystem::exists(dirPath)) {
                    std::filesystem::create_directories(dirPath);
                }
                
                logfilePath = (dirPath / "doowm.log").string();
            } catch (const std::exception& e) {
                std::cerr << "Failed to create log directory: " << e.what() << std::endl;
                logfilePath = "";
            }
        }
    }
    
    // 初期化フラグを先に設定して再帰を防ぐ
    initialized = true;
    
    if (!logfilePath.empty()) {
        setLogFile(logfilePath);
    }
    
    log("Logger initialized with level: " + levelToString(level) + 
        (logfilePath.empty() ? "" : ", log file: " + logfilePath));
}

void Logger::setLevel(LogLevel level) {
    currentLevel = level;
    log("Log level changed to: " + levelToString(level));
}

bool Logger::setLogFile(const std::string& logfile) {
    if (logFile) {
        logFile->close();
    }
    
    logFile = std::make_unique<std::ofstream>(logfile, std::ios::app);
    
    if (!logFile->is_open()) {
        std::cerr << "Failed to open log file: " << logfile << std::endl;
        logFile = nullptr;
        return false;
    }
    
    log("Log file set to: " + logfile);
    return true;
}

void Logger::log(const std::string& message, LogLevel level) {
    if (!initialized) {
        initialized = true;  // 先にフラグを設定
        init();
    }
    
    if (level < currentLevel) {
        return;
    }
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    std::string levelStr = levelToString(level);
    std::string logEntry = "[" + ss.str() + "] [" + levelStr + "] " + message;
    
    // Output to console
    std::cout << logEntry << std::endl;
    
    // Output to file if available
    if (logFile && logFile->is_open()) {
        *logFile << logEntry << std::endl;
        logFile->flush();
    }
}

void Logger::debug(const std::string& message) {
    log(message, LogLevel::DEBUG);
}

void Logger::info(const std::string& message) {
    log(message, LogLevel::INFO);
}

void Logger::warning(const std::string& message) {
    log(message, LogLevel::WARNING);
}

void Logger::error(const std::string& message) {
    log(message, LogLevel::ERROR);
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
} 