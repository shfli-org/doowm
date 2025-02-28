#include "x/x.h"
#include "log/logger.h"
#include <memory>
#include <stdexcept>

int main(int argc, char** argv) {
    // Initialize the logger with default settings
    // This will use DEBUG level and the default log file at ~/.config/doowm/doowm.log
    Logger::init();
    Logger::log("Starting window manager...");

    try {
        Logger::debug("Creating X instance");
        auto x = std::make_unique<X::X>();
        
        Logger::debug("Initializing X system");
        if (!x->initialize()) {
            Logger::error("Failed to initialize X system");
            return 1;
        }
        
        Logger::debug("Starting main event loop");
        x->run();
        
        Logger::log("Window manager shutting down normally");
        return 0;
    } catch (const std::exception& e) {
        Logger::error("Fatal error: " + std::string(e.what()));
        return 1;
    } catch (...) {
        Logger::error("Unknown fatal error occurred");
        return 1;
    }
} 