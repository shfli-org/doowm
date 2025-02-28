#include "window.h"
#include "../../log/logger.h"
#include <xcb/xcb_icccm.h>

namespace X {

Window::Window(Connection& connection, xcb_window_t windowId)
    : connection(connection), windowId(windowId), created(false) {
    Logger::debug("Managing existing window: " + std::to_string(windowId));
    initialize();
}

Window::Window(Connection& connection, int x, int y, unsigned int width, 
               unsigned int height, unsigned int borderWidth)
    : connection(connection), created(true) {
    
    // Generate a new window ID
    windowId = connection.generateId();
    
    // Create the window
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2];
    values[0] = connection.getScreen()->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS |
                XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    
    xcb_create_window(
        connection.getConnection(),
        XCB_COPY_FROM_PARENT,           // depth
        windowId,                        // window id
        connection.getRootWindow(),      // parent window
        x, y,                            // x, y
        width, height,                   // width, height
        borderWidth,                     // border width
        XCB_WINDOW_CLASS_INPUT_OUTPUT,   // class
        connection.getScreen()->root_visual, // visual
        mask, values                     // masks
    );
    
    Logger::debug("Created new window: " + std::to_string(windowId));
    initialize();
}

Window::~Window() {
    if (created) {
        Logger::debug("Destroying window: " + std::to_string(windowId));
        xcb_destroy_window(connection.getConnection(), windowId);
        connection.flush();
    }
}

void Window::initialize() {
    // Set default properties
    setBorderColor(0x000000); // Black border
}

void Window::map() {
    Logger::debug("Mapping window: " + std::to_string(windowId));
    xcb_map_window(connection.getConnection(), windowId);
    connection.flush();
}

void Window::unmap() {
    Logger::debug("Unmapping window: " + std::to_string(windowId));
    xcb_unmap_window(connection.getConnection(), windowId);
    connection.flush();
}

void Window::configure(int x, int y, unsigned int width, unsigned int height, 
                      unsigned int borderWidth, uint32_t stackMode) {
    uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                    XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT |
                    XCB_CONFIG_WINDOW_BORDER_WIDTH | XCB_CONFIG_WINDOW_STACK_MODE;
    
    uint32_t values[6];
    values[0] = x;
    values[1] = y;
    values[2] = width;
    values[3] = height;
    values[4] = borderWidth;
    values[5] = stackMode;
    
    xcb_configure_window(connection.getConnection(), windowId, mask, values);
    connection.flush();
    
    Logger::debug("Configured window " + std::to_string(windowId) + 
                 " to x=" + std::to_string(x) + 
                 ", y=" + std::to_string(y) + 
                 ", width=" + std::to_string(width) + 
                 ", height=" + std::to_string(height) + 
                 ", border=" + std::to_string(borderWidth));
}

void Window::move(int x, int y) {
    uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    uint32_t values[2];
    values[0] = x;
    values[1] = y;
    
    xcb_configure_window(connection.getConnection(), windowId, mask, values);
    connection.flush();
    
    Logger::debug("Moved window " + std::to_string(windowId) + 
                 " to x=" + std::to_string(x) + 
                 ", y=" + std::to_string(y));
}

void Window::resize(unsigned int width, unsigned int height) {
    uint32_t mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    uint32_t values[2];
    values[0] = width;
    values[1] = height;
    
    xcb_configure_window(connection.getConnection(), windowId, mask, values);
    connection.flush();
    
    Logger::debug("Resized window " + std::to_string(windowId) + 
                 " to width=" + std::to_string(width) + 
                 ", height=" + std::to_string(height));
}

void Window::setBorderWidth(unsigned int width) {
    uint32_t mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
    uint32_t values[1];
    values[0] = width;
    
    xcb_configure_window(connection.getConnection(), windowId, mask, values);
    connection.flush();
    
    Logger::debug("Set border width of window " + std::to_string(windowId) + 
                 " to " + std::to_string(width));
}

void Window::setBorderColor(uint32_t color) {
    uint32_t mask = XCB_CW_BORDER_PIXEL;
    uint32_t values[1];
    values[0] = color;
    
    xcb_change_window_attributes(connection.getConnection(), windowId, mask, values);
    connection.flush();
    
    Logger::debug("Set border color of window " + std::to_string(windowId) + 
                 " to 0x" + std::to_string(color));
}

void Window::focus() {
    // Set input focus to this window
    xcb_set_input_focus(
        connection.getConnection(),
        XCB_INPUT_FOCUS_POINTER_ROOT,
        windowId,
        XCB_CURRENT_TIME
    );
    
    // Raise the window to the top
    raise();
    
    connection.flush();
    
    Logger::debug("Focused window: " + std::to_string(windowId));
}

void Window::raise() {
    uint32_t mask = XCB_CONFIG_WINDOW_STACK_MODE;
    uint32_t values[1];
    values[0] = XCB_STACK_MODE_ABOVE;
    
    xcb_configure_window(connection.getConnection(), windowId, mask, values);
    connection.flush();
    
    Logger::debug("Raised window: " + std::to_string(windowId));
}

void Window::lower() {
    uint32_t mask = XCB_CONFIG_WINDOW_STACK_MODE;
    uint32_t values[1];
    values[0] = XCB_STACK_MODE_BELOW;
    
    xcb_configure_window(connection.getConnection(), windowId, mask, values);
    connection.flush();
    
    Logger::debug("Lowered window: " + std::to_string(windowId));
}

std::string Window::getName() {
    return connection.getWindowName(windowId);
}

bool Window::getGeometry(int& x, int& y, unsigned int& width, 
                        unsigned int& height, unsigned int& borderWidth) {
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(
        connection.getConnection(),
        windowId
    );
    
    xcb_get_geometry_reply_t* reply = xcb_get_geometry_reply(
        connection.getConnection(),
        cookie,
        nullptr
    );
    
    if (!reply) {
        Logger::warning("Failed to get geometry for window " + std::to_string(windowId));
        return false;
    }
    
    x = reply->x;
    y = reply->y;
    width = reply->width;
    height = reply->height;
    borderWidth = reply->border_width;
    
    free(reply);
    return true;
}

bool Window::shouldManage(Connection& connection, xcb_window_t windowId) {
    // Get window attributes to check if it's viewable
    xcb_get_window_attributes_cookie_t attr_cookie = 
        xcb_get_window_attributes(connection.getConnection(), windowId);
    
    xcb_get_window_attributes_reply_t* attr_reply = 
        xcb_get_window_attributes_reply(connection.getConnection(), attr_cookie, nullptr);
    
    if (!attr_reply) {
        return false;
    }
    
    // Check if the window is already mapped (viewable)
    bool viewable = attr_reply->map_state == XCB_MAP_STATE_VIEWABLE;
    bool override_redirect = attr_reply->override_redirect;
    free(attr_reply);
    
    // Don't manage windows with override_redirect set
    if (override_redirect) {
        return false;
    }
    
    // Get window type
    xcb_get_property_cookie_t type_cookie = xcb_get_property(
        connection.getConnection(),
        0,
        windowId,
        XCB_ATOM_WM_CLASS,
        XCB_ATOM_STRING,
        0,
        1024
    );
    
    xcb_get_property_reply_t* type_reply = 
        xcb_get_property_reply(connection.getConnection(), type_cookie, nullptr);
    
    bool should_manage = true;
    
    if (type_reply) {
        // Check window class to exclude certain windows
        if (type_reply->type == XCB_ATOM_STRING && type_reply->format == 8) {
            const char* class_str = (const char*)xcb_get_property_value(type_reply);
            int len = xcb_get_property_value_length(type_reply);
            std::string window_class(class_str, len);
            
            // Don't manage desktop, dock, or other special windows
            if (window_class.find("desktop") != std::string::npos ||
                window_class.find("dock") != std::string::npos) {
                should_manage = false;
            }
        }
        free(type_reply);
    }
    
    return viewable && should_manage;
}

} // namespace X
