#include "launcher.h"
#include "../../log/logger.h"
#include <xcb/xcb_aux.h>
#include <xcb/xcb_icccm.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace X {

Launcher::Launcher(Connection& connection)
    : connection(connection), visible(false) {
    createWindow();
    Logger::debug("Launcher initialized");
}

Launcher::~Launcher() {
    // Destroy the launcher window
    if (window) {
        xcb_destroy_window(connection.getConnection(), window);
        connection.flush();
    }
    Logger::debug("Launcher destroyed");
}

void Launcher::show() {
    if (!visible) {
        // Clear the command
        command.clear();
        
        // Map the window
        xcb_map_window(connection.getConnection(), window);
        
        // Set input focus to the launcher window
        xcb_set_input_focus(
            connection.getConnection(),
            XCB_INPUT_FOCUS_POINTER_ROOT,
            window,
            XCB_CURRENT_TIME
        );
        
        // Raise the window to the top
        uint32_t values[1] = { XCB_STACK_MODE_ABOVE };
        xcb_configure_window(
            connection.getConnection(),
            window,
            XCB_CONFIG_WINDOW_STACK_MODE,
            values
        );
        
        connection.flush();
        visible = true;
        
        // Draw the initial state
        draw();
        
        Logger::info("Launcher shown");
    }
}

void Launcher::hide() {
    if (visible) {
        // Unmap the window
        xcb_unmap_window(connection.getConnection(), window);
        connection.flush();
        visible = false;
        Logger::info("Launcher hidden");
    }
}

bool Launcher::isVisible() const {
    return visible;
}

bool Launcher::handleKeyPress(xcb_key_press_event_t* event) {
    if (!visible) {
        return false;
    }
    
    // Check if this is for our window
    if (event->event != window) {
        return false;
    }
    
    // Handle key press
    switch (event->detail) {
        case 36: // Return key
            executeCommand();
            hide();
            return true;
            
        case 9: // Escape key
            hide();
            return true;
            
        case 22: // Backspace key
            if (!command.empty()) {
                command.pop_back();
                draw();
            }
            return true;
            
        default: {
            // Convert keycode to character (simplified)
            char key = 0;
            
            // Very basic keycode to ASCII mapping for demonstration
            // In a real implementation, you would use XKB or similar
            if (event->detail >= 10 && event->detail <= 19) {
                // Number keys 1-0
                key = '0' + ((event->detail - 10 + 1) % 10);
            } else if (event->detail >= 24 && event->detail <= 33) {
                // Letter keys Q-P
                key = 'a' + (event->detail - 24);
            } else if (event->detail >= 38 && event->detail <= 46) {
                // Letter keys A-L
                key = 'a' + (event->detail - 38 + 10);
            } else if (event->detail >= 52 && event->detail <= 58) {
                // Letter keys Z-M
                key = 'a' + (event->detail - 52 + 19);
            } else if (event->detail == 65) {
                // Space
                key = ' ';
            }
            
            if (key) {
                command += key;
                draw();
                return true;
            }
            break;
        }
    }
    
    return false;
}

void Launcher::setExecuteCallback(std::function<void(const std::string&)> callback) {
    executeCallback = callback;
}

void Launcher::createWindow() {
    // Generate a new window ID
    window = connection.generateId();
    
    // Get the root window
    xcb_window_t rootWindow = connection.getRootWindow();
    
    // Set window attributes
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2];
    values[0] = connection.getScreen()->white_pixel;  // Background color
    values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;  // Events to receive
    
    // Create the window
    xcb_create_window(
        connection.getConnection(),
        XCB_COPY_FROM_PARENT,  // Depth
        window,                 // Window ID
        rootWindow,             // Parent window
        100, 100,               // x, y position
        400, 50,                // width, height
        1,                      // Border width
        XCB_WINDOW_CLASS_INPUT_OUTPUT,  // Window class
        connection.getScreen()->root_visual,  // Visual
        mask, values            // Attributes
    );
    
    // Set window title
    xcb_change_property(
        connection.getConnection(),
        XCB_PROP_MODE_REPLACE,
        window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        12,
        "Run Command"
    );
    
    // Check if window creation was successful
    auto cookie = xcb_get_window_attributes(connection.getConnection(), window);
    auto reply = xcb_get_window_attributes_reply(connection.getConnection(), cookie, nullptr);
    
    if (!reply) {
        Logger::error("Failed to create launcher window");
        window = 0;  // Set to invalid window ID
        return;
    }
    
    free(reply);
    
    connection.flush();
    Logger::debug("Launcher window created");
}

void Launcher::draw() {
    if (!visible) {
        return;
    }
    
    // Clear the window
    xcb_clear_area(
        connection.getConnection(),
        0,              // exposures
        window,         // window
        0, 0,           // x, y
        0, 0            // width, height (0 means entire window)
    );
    
    // Create a graphics context
    xcb_gcontext_t gc = xcb_generate_id(connection.getConnection());
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    uint32_t values[2];
    values[0] = connection.getScreen()->black_pixel;
    values[1] = connection.getScreen()->white_pixel;
    
    xcb_create_gc(
        connection.getConnection(),
        gc,
        window,
        mask,
        values
    );
    
    // Draw the command text
    std::string displayText = "Run: " + command;
    xcb_image_text_8(
        connection.getConnection(),
        displayText.length(),
        window,
        gc,
        10, 20,         // x, y position for text
        displayText.c_str()
    );
    
    // Free the graphics context
    xcb_free_gc(connection.getConnection(), gc);
    
    connection.flush();
}

void Launcher::executeCommand() {
    if (command.empty()) {
        return;
    }
    
    Logger::info("Executing command: " + command);
    
    // Call the callback if set
    if (executeCallback) {
        executeCallback(command);
    } else {
        // Default implementation: fork and exec
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            
            // Close X connection in the child
            connection.close();
            
            // Execute the command
            // Note: This is a very simple implementation
            // In a real application, you would want to parse the command properly
            execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
            
            // If execl returns, there was an error
            exit(1);
        } else if (pid < 0) {
            // Fork failed
            Logger::error("Failed to fork process for command: " + command);
        } else {
            // Parent process
            // We don't wait for the child to complete
            Logger::debug("Launched command with PID: " + std::to_string(pid));
        }
    }
}

} // namespace X 