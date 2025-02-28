#include "event_handler.h"
#include "../../log/logger.h"
#include "../window/window.h"
#include <xcb/xcb.h>
#include <sstream>

namespace X {

EventHandler::EventHandler(X& system)
    : system(system) {
    Logger::debug("Event handler initialized");
}

void EventHandler::processNextEvent() {
    xcb_generic_event_t* event = xcb_wait_for_event(system.getConnection().getConnection());
    
    if (!event) {
        Logger::warning("Failed to get next event, connection might be broken");
        system.terminate();
        return;
    }
    
    // Get the event type, masking out the high bits
    uint8_t eventType = event->response_type & ~0x80;
    
    // Process the event based on its type
    switch (eventType) {
        case XCB_MAP_REQUEST: {
            auto mapEvent = reinterpret_cast<xcb_map_request_event_t*>(event);
            handleMapRequest(mapEvent);
            break;
        }
        case XCB_CONFIGURE_REQUEST: {
            auto configEvent = reinterpret_cast<xcb_configure_request_event_t*>(event);
            handleConfigureRequest(configEvent);
            break;
        }
        case XCB_UNMAP_NOTIFY: {
            auto unmapEvent = reinterpret_cast<xcb_unmap_notify_event_t*>(event);
            handleUnmapNotify(unmapEvent);
            break;
        }
        case XCB_DESTROY_NOTIFY: {
            auto destroyEvent = reinterpret_cast<xcb_destroy_notify_event_t*>(event);
            handleDestroyNotify(destroyEvent);
            break;
        }
        case XCB_KEY_PRESS: {
            auto keyEvent = reinterpret_cast<xcb_key_press_event_t*>(event);
            handleKeyPress(keyEvent);
            break;
        }
        case XCB_BUTTON_PRESS: {
            auto buttonEvent = reinterpret_cast<xcb_button_press_event_t*>(event);
            handleButtonPress(buttonEvent);
            break;
        }
        case XCB_BUTTON_RELEASE: {
            auto buttonEvent = reinterpret_cast<xcb_button_release_event_t*>(event);
            handleButtonRelease(buttonEvent);
            break;
        }
        case XCB_MOTION_NOTIFY: {
            auto motionEvent = reinterpret_cast<xcb_motion_notify_event_t*>(event);
            handleMotionNotify(motionEvent);
            break;
        }
        default:
            // Log unhandled event types for debugging
            Logger::debug("Unhandled event type: " + std::to_string(eventType));
            break;
    }
    
    // Free the event
    free(event);
}

void EventHandler::handleMapRequest(xcb_map_request_event_t* event) {
    Logger::debug("Map request for window: " + std::to_string(event->window));
    
    // Check if we should manage this window
    if (Window::shouldManage(system.getConnection(), event->window)) {
        // Create a window object for the new window
        auto window = new Window(system.getConnection(), event->window);
        
        // Set border width and color
        window->setBorderWidth(2);
        window->setBorderColor(0x3388FF); // Blue border for managed windows
        
        // Map the window
        window->map();
        
        // Focus the new window
        window->focus();
        
        Logger::info("New window managed: " + std::to_string(event->window));
    } else {
        // Just map the window without managing it
        xcb_map_window(system.getConnection().getConnection(), event->window);
        system.getConnection().flush();
        
        Logger::debug("Window mapped but not managed: " + std::to_string(event->window));
    }
}

void EventHandler::handleConfigureRequest(xcb_configure_request_event_t* event) {
    Logger::debug("Configure request for window: " + std::to_string(event->window));
    
    // Prepare values to be configured
    uint16_t mask = event->value_mask;
    uint32_t values[7];
    unsigned int i = 0;
    
    if (mask & XCB_CONFIG_WINDOW_X) {
        values[i++] = event->x;
    }
    if (mask & XCB_CONFIG_WINDOW_Y) {
        values[i++] = event->y;
    }
    if (mask & XCB_CONFIG_WINDOW_WIDTH) {
        values[i++] = event->width;
    }
    if (mask & XCB_CONFIG_WINDOW_HEIGHT) {
        values[i++] = event->height;
    }
    if (mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
        values[i++] = event->border_width;
    }
    if (mask & XCB_CONFIG_WINDOW_SIBLING) {
        values[i++] = event->sibling;
    }
    if (mask & XCB_CONFIG_WINDOW_STACK_MODE) {
        values[i++] = event->stack_mode;
    }
    
    // Apply the configuration
    xcb_configure_window(
        system.getConnection().getConnection(),
        event->window,
        mask,
        values
    );
    
    system.getConnection().flush();
}

void EventHandler::handleUnmapNotify(xcb_unmap_notify_event_t* event) {
    Logger::debug("Unmap notify for window: " + std::to_string(event->window));
    
    // Handle window unmapping
    // In a more complete implementation, we would remove the window from our managed windows list
}

void EventHandler::handleDestroyNotify(xcb_destroy_notify_event_t* event) {
    Logger::debug("Destroy notify for window: " + std::to_string(event->window));
    
    // Handle window destruction
    // In a more complete implementation, we would remove the window from our managed windows list
}

void EventHandler::handleKeyPress(xcb_key_press_event_t* event) {
    std::stringstream ss;
    ss << "Key press event: "
       << "keycode=" << static_cast<int>(event->detail)
       << ", modifiers=0x" << std::hex << event->state
       << ", window=0x" << std::hex << event->event
       << ", root=0x" << std::hex << event->root
       << ", time=" << std::dec << event->time
       << ", root_x=" << event->root_x
       << ", root_y=" << event->root_y
       << ", event_x=" << event->event_x
       << ", event_y=" << event->event_y;
    
    Logger::info(ss.str());
    
    // Handle keyboard shortcuts
    // Example: Alt+F4 to close a window
    if (event->detail == 70 && (event->state & XCB_MOD_MASK_1)) {
        Logger::info("Alt+F4 pressed - Attempting to close window");
        
        // In a real implementation, we would send a close request to the window
        // For now, just log the action
    }
    
    // Example: Alt+Tab to switch windows
    if (event->detail == 23 && (event->state & XCB_MOD_MASK_1)) {
        Logger::info("Alt+Tab pressed - Switching windows");
        
        // In a real implementation, we would cycle through managed windows
        // For now, just log the action
    }
}

void EventHandler::handleButtonPress(xcb_button_press_event_t* event) {
    std::stringstream ss;
    ss << "Button press event: "
       << "button=" << static_cast<int>(event->detail)
       << ", modifiers=0x" << std::hex << event->state
       << ", window=0x" << std::hex << event->event
       << ", root=0x" << std::hex << event->root
       << ", time=" << std::dec << event->time
       << ", root_x=" << event->root_x
       << ", root_y=" << event->root_y
       << ", event_x=" << event->event_x
       << ", event_y=" << event->event_y;
    
    Logger::info(ss.str());
    
    // Handle mouse button events
    switch (event->detail) {
        case 1: { // Left button
            Logger::info("Left mouse button pressed on window 0x" + 
                        std::to_string(event->event));
            
            // Focus the clicked window
            xcb_set_input_focus(
                system.getConnection().getConnection(),
                XCB_INPUT_FOCUS_POINTER_ROOT,
                event->event,
                XCB_CURRENT_TIME
            );
            
            // Raise the window to the top
            uint32_t values[1] = { XCB_STACK_MODE_ABOVE };
            xcb_configure_window(
                system.getConnection().getConnection(),
                event->event,
                XCB_CONFIG_WINDOW_STACK_MODE,
                values
            );
            
            system.getConnection().flush();
            break;
        }
            
        case 2: { // Middle button
            Logger::info("Middle mouse button pressed on window 0x" + 
                        std::to_string(event->event));
            break;
        }
            
        case 3: { // Right button
            Logger::info("Right mouse button pressed on window 0x" + 
                        std::to_string(event->event));
            break;
        }
            
        case 4: { // Scroll up
            Logger::info("Scroll up on window 0x" + 
                        std::to_string(event->event));
            break;
        }
            
        case 5: { // Scroll down
            Logger::info("Scroll down on window 0x" + 
                        std::to_string(event->event));
            break;
        }
            
        default: {
            Logger::info("Button " + std::to_string(event->detail) + 
                        " pressed on window 0x" + std::to_string(event->event));
            break;
        }
    }
}

void EventHandler::handleButtonRelease(xcb_button_release_event_t* event) {
    std::stringstream ss;
    ss << "Button release event: "
       << "button=" << static_cast<int>(event->detail)
       << ", modifiers=0x" << std::hex << event->state
       << ", window=0x" << std::hex << event->event
       << ", root=0x" << std::hex << event->root
       << ", time=" << std::dec << event->time
       << ", root_x=" << event->root_x
       << ", root_y=" << event->root_y
       << ", event_x=" << event->event_x
       << ", event_y=" << event->event_y;
    
    Logger::info(ss.str());
}

void EventHandler::handleMotionNotify(xcb_motion_notify_event_t* event) {
    // Motion events can be very frequent, so we might want to log them at DEBUG level
    // or only log them when certain conditions are met
    
    std::stringstream ss;
    ss << "Motion notify event: "
       << "window=0x" << std::hex << event->event
       << ", root=0x" << std::hex << event->root
       << ", time=" << std::dec << event->time
       << ", root_x=" << event->root_x
       << ", root_y=" << event->root_y
       << ", event_x=" << event->event_x
       << ", event_y=" << event->event_y;
    
    // Only log motion events when a button is pressed (dragging)
    if (event->state & (XCB_BUTTON_MASK_1 | XCB_BUTTON_MASK_2 | XCB_BUTTON_MASK_3)) {
        Logger::debug(ss.str());
    }
}

} // namespace X
