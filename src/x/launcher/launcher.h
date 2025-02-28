#pragma once

#include "../connection/connection.h"
#include <string>
#include <functional>
#include <xcb/xcb.h>

namespace X {

/**
 * @class Launcher
 * @brief Simple application launcher
 * 
 * Provides a simple dialog for launching applications
 */
class Launcher {
public:
    /**
     * @brief Constructor
     * @param connection The X connection
     */
    Launcher(Connection& connection);
    
    /**
     * @brief Destructor
     */
    ~Launcher();
    
    /**
     * @brief Show the launcher dialog
     */
    void show();
    
    /**
     * @brief Hide the launcher dialog
     */
    void hide();
    
    /**
     * @brief Check if the launcher is visible
     * @return true if visible, false otherwise
     */
    bool isVisible() const;
    
    /**
     * @brief Handle a key press event
     * @param event The key press event
     * @return true if the event was handled, false otherwise
     */
    bool handleKeyPress(xcb_key_press_event_t* event);
    
    /**
     * @brief Set the callback function for command execution
     * @param callback The function to call when a command is executed
     */
    void setExecuteCallback(std::function<void(const std::string&)> callback);

private:
    Connection& connection;
    xcb_window_t window;
    bool visible;
    std::string command;
    std::function<void(const std::string&)> executeCallback;
    
    /**
     * @brief Create the launcher window
     */
    void createWindow();
    
    /**
     * @brief Draw the launcher window
     */
    void draw();
    
    /**
     * @brief Execute the current command
     */
    void executeCommand();
};

} // namespace X 