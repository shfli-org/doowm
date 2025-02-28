#pragma once

#include "../connection/connection.h"
#include <xcb/xcb.h>
#include <string>

namespace X {

/**
 * @class Window
 * @brief Wrapper for XCB window
 * 
 * Provides methods for manipulating X windows.
 */
class Window {
public:
    /**
     * @brief Constructor for existing window
     * @param connection The X connection
     * @param windowId The ID of the existing window
     */
    Window(Connection& connection, xcb_window_t windowId);
    
    /**
     * @brief Constructor for creating a new window
     * @param connection The X connection
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param borderWidth Border width
     */
    Window(Connection& connection, int x, int y, unsigned int width, 
           unsigned int height, unsigned int borderWidth = 1);
    
    /**
     * @brief Destructor
     */
    ~Window();
    
    /**
     * @brief Get the window ID
     * @return The window ID
     */
    xcb_window_t getId() const { return windowId; }
    
    /**
     * @brief Map (show) the window
     */
    void map();
    
    /**
     * @brief Unmap (hide) the window
     */
    void unmap();
    
    /**
     * @brief Configure window properties
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param borderWidth Border width
     * @param stackMode Stack mode (above, below, etc.)
     */
    void configure(int x, int y, unsigned int width, unsigned int height, 
                  unsigned int borderWidth, uint32_t stackMode = XCB_STACK_MODE_ABOVE);
    
    /**
     * @brief Move the window
     * @param x New X position
     * @param y New Y position
     */
    void move(int x, int y);
    
    /**
     * @brief Resize the window
     * @param width New width
     * @param height New height
     */
    void resize(unsigned int width, unsigned int height);
    
    /**
     * @brief Set the border width
     * @param width New border width
     */
    void setBorderWidth(unsigned int width);
    
    /**
     * @brief Set the border color
     * @param color The color value
     */
    void setBorderColor(uint32_t color);
    
    /**
     * @brief Focus this window
     */
    void focus();
    
    /**
     * @brief Raise this window to the top of the stack
     */
    void raise();
    
    /**
     * @brief Lower this window to the bottom of the stack
     */
    void lower();
    
    /**
     * @brief Get the window name
     * @return The window name
     */
    std::string getName();
    
    /**
     * @brief Get the window geometry
     * @param x Output parameter for X position
     * @param y Output parameter for Y position
     * @param width Output parameter for width
     * @param height Output parameter for height
     * @param borderWidth Output parameter for border width
     * @return true if successful, false otherwise
     */
    bool getGeometry(int& x, int& y, unsigned int& width, 
                    unsigned int& height, unsigned int& borderWidth);
    
    /**
     * @brief Determine if a window should be managed by the window manager
     * @param connection The X connection
     * @param windowId The window ID to check
     * @return true if the window should be managed, false otherwise
     */
    static bool shouldManage(Connection& connection, xcb_window_t windowId);

private:
    Connection& connection;
    xcb_window_t windowId;
    bool created;  // Whether this window was created by us or is existing
    
    /**
     * @brief Initialize window attributes
     */
    void initialize();
};

} // namespace X