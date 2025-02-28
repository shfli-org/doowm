#pragma once
#include <xcb/xcb.h>
#include "../x.h"

namespace X {

class X;

class EventHandler {
public:
    EventHandler(X& system);
    void processNextEvent();
    
private:
    X& system;
    
    void handleMapRequest(xcb_map_request_event_t* event);
    void handleConfigureRequest(xcb_configure_request_event_t* event);
    void handleUnmapNotify(xcb_unmap_notify_event_t* event);
    void handleDestroyNotify(xcb_destroy_notify_event_t* event);
    void handleKeyPress(xcb_key_press_event_t* event);
    void handleButtonPress(xcb_button_press_event_t* event);
    void handleButtonRelease(xcb_button_release_event_t* event);
    void handleMotionNotify(xcb_motion_notify_event_t* event);
};

} // namespace X 