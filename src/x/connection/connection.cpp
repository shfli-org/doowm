#include "connection.h"
#include "../../log/logger.h"
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <stdexcept>

namespace X {

Connection::Connection(const char* displayName) {
    Logger::debug("Connecting to X server" + 
                 (displayName ? std::string(": ") + displayName : std::string("")));
    
    // Connect to the X server
    connection = xcb_connect(displayName, &screenNum);
    
    // Check for connection errors
    int error = xcb_connection_has_error(connection);
    if (error) {
        std::string errorMsg = "Failed to connect to X server: ";
        switch (error) {
            case XCB_CONN_ERROR:
                errorMsg += "Connection error";
                break;
            case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
                errorMsg += "Extension not supported";
                break;
            case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
                errorMsg += "Insufficient memory";
                break;
            case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
                errorMsg += "Request length exceeded";
                break;
            case XCB_CONN_CLOSED_PARSE_ERR:
                errorMsg += "Parse error";
                break;
            case XCB_CONN_CLOSED_INVALID_SCREEN:
                errorMsg += "Invalid screen";
                break;
            default:
                errorMsg += "Unknown error";
                break;
        }
        
        Logger::error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    
    // Get the screen
    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    
    // Advance to the screen we want
    for (int i = 0; i < screenNum; ++i) {
        xcb_screen_next(&iter);
    }
    
    screen = iter.data;
    
    if (!screen) {
        Logger::error("Failed to get screen information");
        throw std::runtime_error("Failed to get screen information");
    }
    
    Logger::info("Connected to X server, screen: " + std::to_string(screenNum) + 
                ", dimensions: " + std::to_string(screen->width_in_pixels) + "x" + 
                std::to_string(screen->height_in_pixels));
}

Connection::~Connection() {
    Logger::debug("Disconnecting from X server");
    if (connection) {
        xcb_disconnect(connection);
        connection = nullptr;
    }
}

bool Connection::isConnected() const {
    return connection && (xcb_connection_has_error(connection) == 0);
}

void Connection::flush() {
    xcb_flush(connection);
}

xcb_window_t Connection::generateId() {
    return xcb_generate_id(connection);
}

std::string Connection::getWindowName(xcb_window_t window) {
    // Try to get _NET_WM_NAME (UTF-8)
    xcb_get_property_cookie_t cookie = xcb_get_property(
        connection,
        0,
        window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        0,
        1024
    );
    
    xcb_get_property_reply_t* reply = xcb_get_property_reply(
        connection,
        cookie,
        nullptr
    );
    
    if (!reply) {
        return "";
    }
    
    std::string name;
    if (reply->type == XCB_ATOM_STRING && reply->format == 8) {
        const char* value = (const char*)xcb_get_property_value(reply);
        int len = xcb_get_property_value_length(reply);
        name = std::string(value, len);
    }
    
    free(reply);
    return name;
}

xcb_connection_t* Connection::getConnection() const {
    return connection;
}

xcb_window_t Connection::getRootWindow() const {
    return screen->root;
}

void Connection::close() {
    if (connection) {
        xcb_disconnect(connection);
        connection = nullptr;
    }
}

} // namespace X 