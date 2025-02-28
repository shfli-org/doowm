#pragma once

#include "../connection/connection.h"
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <map>
#include <functional>

namespace X {
namespace Keyboard {

/**
 * @class KeyboardHandler
 * @brief Manages keyboard input and shortcuts for the window manager
 * 
 * This class is responsible for grabbing keys for window management shortcuts
 * and handling key press events.
 */
class KeyboardHandler {
public:
    /**
     * @brief Constructor
     * @param connection The X connection
     */
    KeyboardHandler(Connection& connection);
    
    /**
     * @brief Destructor
     */
    ~KeyboardHandler();
    
    /**
     * @brief Grab keys used for window management shortcuts
     */
    void grabWMKeys();
    
    /**
     * @brief Handle a key press event
     * @param event The key press event
     * @return true if the key was handled, false otherwise
     */
    bool handleKeyPress(xcb_key_press_event_t* event);
    
    /**
     * @brief Register a callback for a key combination
     * @param keycode The keycode to register
     * @param modifiers The modifier mask (Alt, Ctrl, etc.)
     * @param callback The function to call when the key is pressed
     */
    void registerKeyCallback(uint8_t keycode, uint16_t modifiers, 
                            std::function<void()> callback);

private:
    Connection& connection;
    xcb_key_symbols_t* keySymbols;
    
    // Map of key combinations to callback functions
    std::map<std::pair<uint8_t, uint16_t>, std::function<void()>> keyCallbacks;
    
    /**
     * @brief Grab a specific key with modifiers
     * @param keycode The keycode to grab
     * @param modifiers The modifier mask
     */
    void grabKey(uint8_t keycode, uint16_t modifiers);
    
    /**
     * @brief Convert a keysym to a keycode
     * @param keysym The keysym to convert
     * @return The corresponding keycode
     */
    uint8_t keysymToKeycode(xcb_keysym_t keysym);
};

} // namespace Keyboard
} // namespace X
