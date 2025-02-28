#pragma once

#include <memory>
#include <vector>
#include "connection/connection.h"
#include "window/window.h"
#include "keyboard/keyboard.h"
#include "launcher/launcher.h"

namespace X {

class EventHandler;

/**
 * @class X
 * @brief Main class that manages the X window system interaction
 * 
 * This class is responsible for initializing the X connection,
 * setting up the root window, and handling the main event loop.
 * It serves as the central coordinator for the window manager.
 */
class X {
public:
    /**
     * @brief Constructor
     */
    X();
    
    /**
     * @brief Destructor
     */
    ~X();
    
    /**
     * @brief Initialize the X system
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Run the main event loop
     * 
     * This method enters the main event loop and processes X events
     * until terminate() is called.
     */
    void run();
    
    /**
     * @brief Request termination of the event loop
     * 
     * This method sets a flag that causes the main event loop to exit
     * after the current event is processed.
     */
    void terminate();
    
    /**
     * @brief Get the X connection
     * @return Reference to the X connection
     */
    Connection& getConnection() { return *connection; }
    
    /**
     * @brief Get the root window
     * @return Reference to the root window
     */
    Window& getRootWindow() { return *rootWindow; }
    
    /**
     * @brief Show the application launcher
     * 
     * Displays a simple launcher dialog that allows the user to
     * enter and execute commands.
     */
    void showLauncher();

private:
    /**
     * @brief Set up the root window
     * 
     * Configures the root window to receive the necessary events
     * and sets up key bindings.
     */
    void setupRootWindow();
    
    /**
     * @brief Scan for existing windows
     * 
     * Queries the X server for existing windows and manages them.
     */
    void scanExistingWindows();
   
    bool running;                                      // Flag indicating if the event loop is running
    std::unique_ptr<Connection> connection;            // Connection to the X server
    std::unique_ptr<Window> rootWindow;                // The root window
    std::unique_ptr<EventHandler> eventHandler;        // Handler for X events
    std::unique_ptr<Keyboard::KeyboardHandler> keyboardHandler;  // Handler for keyboard input
    
    std::vector<Window*> managedWindows;               // List of windows managed by the window manager
    std::unique_ptr<Launcher> launcher;                // Application launcher
};

} // namespace X
