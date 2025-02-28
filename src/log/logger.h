#pragma once

#include <string>
#include <fstream>
#include <memory>

/**
 * @enum LogLevel
 * @brief Defines the severity levels for logging
 */
enum class LogLevel {
    DEBUG,    // Detailed information for debugging
    INFO,     // General information about program execution
    WARNING,  // Potential issues that don't prevent execution
    ERROR     // Serious problems that may prevent execution
};

/**
 * @class Logger
 * @brief Simple logging utility for the window manager
 * 
 * Provides methods for logging messages at different severity levels.
 * Can output to console and/or a log file.
 */
class Logger {
public:
    /**
     * @brief Constructor
     * @param level The minimum log level to display (optional)
     * @param logfile Path to the log file (optional)
     */
    Logger(LogLevel level = LogLevel::DEBUG, const std::string& logfile = "");
    
    /**
     * @brief Destructor
     */
    ~Logger();
    
    /**
     * @brief Initialize the logger
     * @param level The minimum log level to display (optional)
     * @param logfile Path to the log file (optional)
     */
    static void init(LogLevel level = LogLevel::DEBUG, const std::string& logfile = "");
    
    /**
     * @brief Set the minimum log level
     * @param level The new minimum log level
     */
    static void setLevel(LogLevel level);
    
    /**
     * @brief Set the log file
     * @param logfile Path to the log file
     * @return true if the file was opened successfully, false otherwise
     */
    static bool setLogFile(const std::string& logfile);
    
    /**
     * @brief Log a message with the specified level
     * @param message The message to log
     * @param level The severity level of the message
     */
    static void log(const std::string& message, LogLevel level = LogLevel::INFO);
    
    /**
     * @brief Log a debug message
     * @param message The message to log
     */
    static void debug(const std::string& message);
    
    /**
     * @brief Log an info message
     * @param message The message to log
     */
    static void info(const std::string& message);
    
    /**
     * @brief Log a warning message
     * @param message The message to log
     */
    static void warning(const std::string& message);
    
    /**
     * @brief Log an error message
     * @param message The message to log
     */
    static void error(const std::string& message);

private:
    static bool initialized;
    static LogLevel currentLevel;
    static std::unique_ptr<std::ofstream> logFile;
    
    /**
     * @brief Convert a log level to its string representation
     * @param level The log level to convert
     * @return The string representation of the log level
     */
    static std::string levelToString(LogLevel level);
}; 