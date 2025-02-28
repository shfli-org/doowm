#pragma once

#include <xcb/xcb.h>
#include <string>
#include <memory>

namespace X {

/**
 * @class Connection
 * @brief Wrapper for XCB connection to the X server
 * 
 * Manages the connection to the X server and provides
 * utility methods for interacting with the X server.
 */
class Connection {
public:
    /**
     * @brief Constructor that establishes a connection to the X server
     */
    Connection(const char* displayName = nullptr);
    
    /**
     * @brief Destructor that closes the connection
     */
    ~Connection();
    
    /**
     * @brief Check if the connection is established
     * @return true if connected, false otherwise
     */
    bool isConnected() const;
    
    /**
     * @brief Get the raw XCB connection
     * @return Pointer to the XCB connection
     */
    xcb_connection_t* getConnection() const;
    
    /**
     * @brief Get the ID of the root window
     * @return The root window ID
     */
    xcb_window_t getRootWindow() const;
    
    /**
     * @brief Get the screen information
     * @return Pointer to the screen information
     */
    xcb_screen_t* getScreen() const { return screen; }
    
    /**
     * @brief Flush the connection (send all pending requests)
     */
    void flush();
    
    /**
     * @brief Generate a new XID for a window
     * @return A new XID
     */
    xcb_window_t generateId();
    
    /**
     * @brief Get the name of a window
     * @param window The window ID
     * @return The window name or an empty string if not available
     */
    std::string getWindowName(xcb_window_t window);

    /**
     * @brief Close the connection to the X server
     */
    void close();

private:
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    int screenNum;
};

} // namespace X 