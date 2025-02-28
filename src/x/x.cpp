#include "x.h"
#include "../log/logger.h"
#include "event/event_handler.h"
#include <xcb/xcb.h>
#include <stdexcept>
#include "launcher/launcher.h"
#include <unistd.h>     // fork(), execl(), setsid()
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid()

namespace X {

X::X() 
    : running(false) {
    // Create a logger instance with default settings
    // This will initialize the logger with DEBUG level and the default log file
    static Logger logger;
    Logger::debug("X constructor called");
}

X::~X() {
    Logger::debug("X destructor called");
    
    // Clean up managed windows
    for (auto window : managedWindows) {
        delete window;
    }
    managedWindows.clear();
    
    // Release resources in reverse order of creation
    launcher.reset();
    eventHandler.reset();
    keyboardHandler.reset();
    rootWindow.reset();
    connection.reset();
}

bool X::initialize() {
    Logger::debug("Initializing X");
    
    try {
        // Create X connection
        connection = std::make_unique<Connection>();
        if (!connection->isConnected()) {
            Logger::error("Failed to connect to X server");
            return false;
        }
        
        // Get the root window
        rootWindow = std::make_unique<Window>(*connection, connection->getRootWindow());
        
        // Set up keyboard handler
        keyboardHandler = std::make_unique<Keyboard::KeyboardHandler>(*connection);
        
        // Set up event handler (after keyboard handler)
        eventHandler = std::make_unique<EventHandler>(*this);
        
        // Configure root window
        setupRootWindow();
        
        // Scan for existing windows
        scanExistingWindows();
        
        // Set up launcher
        launcher = std::make_unique<Launcher>(getConnection());
        launcher->setExecuteCallback([this](const std::string& command) {
            Logger::info("Executing command from launcher: " + command);
            // Fork and exec the command
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                setsid();
                execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
                exit(1);
            }
        });
        
        Logger::info("X initialized successfully");
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize X: " + std::string(e.what()));
        return false;
    }
}

void X::run() {
    Logger::info("Starting main event loop");
    running = true;
    
    while (running) {
        // Wait for and process the next event
        eventHandler->processNextEvent();
    }
    
    Logger::info("Main event loop terminated");
}

void X::terminate() {
    Logger::info("Terminating X");
    running = false;
}

void X::setupRootWindow() {
    Logger::debug("Setting up root window");
    
    // Set event mask for root window to receive notifications about
    // new windows, window destruction, and key events
    uint32_t values[1];
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_PROPERTY_CHANGE |
                XCB_EVENT_MASK_KEY_PRESS;
    
    auto cookie = xcb_change_window_attributes_checked(
        connection->getConnection(),
        rootWindow->getId(),
        XCB_CW_EVENT_MASK,
        values
    );
    
    auto error = xcb_request_check(connection->getConnection(), cookie);
    if (error) {
        free(error);
        throw std::runtime_error("Another window manager is already running");
    }
    
    // Grab keys for window management shortcuts
    keyboardHandler->grabWMKeys();
    
    // Make sure changes are applied
    connection->flush();
}

void X::scanExistingWindows() {
    Logger::debug("Scanning for existing windows");
    
    // Query for existing windows
    auto cookie = xcb_query_tree(
        connection->getConnection(),
        rootWindow->getId()
    );
    
    auto reply = xcb_query_tree_reply(
        connection->getConnection(),
        cookie,
        nullptr
    );
    
    if (!reply) {
        Logger::warning("Failed to query existing windows");
        return;
    }
    
    // Get the list of child windows
    auto children = xcb_query_tree_children(reply);
    auto childrenLen = xcb_query_tree_children_length(reply);
    
    Logger::info("Found " + std::to_string(childrenLen) + " existing windows");
    
    // Manage each window
    for (int i = 0; i < childrenLen; i++) {
        auto windowId = children[i];
        
        // Skip windows that shouldn't be managed
        // (like dock, desktop, etc.)
        if (!Window::shouldManage(*connection, windowId)) {
            continue;
        }
        
        // Create and manage the window
        auto window = new Window(*connection, windowId);
        managedWindows.push_back(window);
        
        // Map the window if it's not already mapped
        window->map();
        
        Logger::debug("Managing existing window: " + std::to_string(windowId));
    }
    
    free(reply);
}

void X::showLauncher() {
    if (launcher) {
        launcher->show();
    }
}

} // namespace X
