#include "keyboard.h"
#include "../../log/logger.h"
#include <X11/keysym.h>

namespace X {
namespace Keyboard {

KeyboardHandler::KeyboardHandler(Connection& connection)
    : connection(connection) {
    // Initialize key symbols
    keySymbols = xcb_key_symbols_alloc(connection.getConnection());
    if (!keySymbols) {
        Logger::error("Failed to allocate key symbols");
    }
    
    Logger::debug("Keyboard handler initialized");
}

KeyboardHandler::~KeyboardHandler() {
    // Free key symbols
    if (keySymbols) {
        xcb_key_symbols_free(keySymbols);
        keySymbols = nullptr;
    }
    
    Logger::debug("Keyboard handler destroyed");
}

void KeyboardHandler::grabWMKeys() {
    Logger::debug("Grabbing window management keys");
    
    // Grab Alt+Tab for window switching
    grabKey(keysymToKeycode(XK_Tab), XCB_MOD_MASK_1);
    
    // Grab Alt+F4 for window closing
    grabKey(keysymToKeycode(XK_F4), XCB_MOD_MASK_1);
    
    // Grab Alt+F2 for launching applications
    grabKey(keysymToKeycode(XK_F2), XCB_MOD_MASK_1);
    
    // Grab Alt+Space for window menu
    grabKey(keysymToKeycode(XK_space), XCB_MOD_MASK_1);
    
    // Grab Alt+F1 for main menu
    grabKey(keysymToKeycode(XK_F1), XCB_MOD_MASK_1);
    
    // Grab Alt+Shift+Left/Right for moving windows between workspaces
    grabKey(keysymToKeycode(XK_Left), XCB_MOD_MASK_1 | XCB_MOD_MASK_SHIFT);
    grabKey(keysymToKeycode(XK_Right), XCB_MOD_MASK_1 | XCB_MOD_MASK_SHIFT);
    
    // Grab Alt+Left/Right for switching workspaces
    grabKey(keysymToKeycode(XK_Left), XCB_MOD_MASK_1);
    grabKey(keysymToKeycode(XK_Right), XCB_MOD_MASK_1);
    
    // Register default callbacks
    registerKeyCallback(keysymToKeycode(XK_Tab), XCB_MOD_MASK_1, [this]() {
        Logger::info("Alt+Tab pressed - Switch window");
    });
    
    registerKeyCallback(keysymToKeycode(XK_F4), XCB_MOD_MASK_1, [this]() {
        Logger::info("Alt+F4 pressed - Close window");
    });
    
    registerKeyCallback(keysymToKeycode(XK_F2), XCB_MOD_MASK_1, [this]() {
        Logger::info("Alt+F2 pressed - Launch application");
    });
    
    // Make sure changes are applied
    connection.flush();
}

bool KeyboardHandler::handleKeyPress(xcb_key_press_event_t* event) {
    if (!event) {
        return false;
    }
    
    // Create a key identifier from keycode and modifiers
    std::pair<uint8_t, uint16_t> keyId(event->detail, event->state & 0xFF);
    
    // Look up the callback for this key combination
    auto it = keyCallbacks.find(keyId);
    if (it != keyCallbacks.end()) {
        // Execute the callback
        it->second();
        return true;
    }
    
    return false;
}

void KeyboardHandler::registerKeyCallback(uint8_t keycode, uint16_t modifiers, 
                                         std::function<void()> callback) {
    // Create a key identifier from keycode and modifiers
    std::pair<uint8_t, uint16_t> keyId(keycode, modifiers);
    
    // Register the callback
    keyCallbacks[keyId] = callback;
    
    Logger::debug("Registered callback for keycode " + std::to_string(keycode) + 
                 " with modifiers " + std::to_string(modifiers));
}

void KeyboardHandler::grabKey(uint8_t keycode, uint16_t modifiers) {
    if (!keycode) {
        Logger::warning("Attempted to grab invalid keycode 0");
        return;
    }
    
    // Grab the key
    xcb_grab_key(
        connection.getConnection(),
        1,                          // owner_events (pass through to window)
        connection.getRootWindow(), // grab_window (root window)
        modifiers,                  // modifiers
        keycode,                    // key
        XCB_GRAB_MODE_ASYNC,        // pointer_mode
        XCB_GRAB_MODE_ASYNC         // keyboard_mode
    );
    
    // Also grab with Num Lock mask
    xcb_grab_key(
        connection.getConnection(),
        1,
        connection.getRootWindow(),
        modifiers | XCB_MOD_MASK_2, // Num Lock mask
        keycode,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC
    );
    
    // Also grab with Caps Lock mask
    xcb_grab_key(
        connection.getConnection(),
        1,
        connection.getRootWindow(),
        modifiers | XCB_MOD_MASK_LOCK, // Caps Lock mask
        keycode,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC
    );
    
    // Also grab with both Num Lock and Caps Lock masks
    xcb_grab_key(
        connection.getConnection(),
        1,
        connection.getRootWindow(),
        modifiers | XCB_MOD_MASK_2 | XCB_MOD_MASK_LOCK,
        keycode,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC
    );
    
    Logger::debug("Grabbed keycode " + std::to_string(keycode) + 
                 " with modifiers " + std::to_string(modifiers));
}

uint8_t KeyboardHandler::keysymToKeycode(xcb_keysym_t keysym) {
    if (!keySymbols) {
        Logger::error("Key symbols not initialized");
        return 0;
    }
    
    xcb_keycode_t* keycode = xcb_key_symbols_get_keycode(keySymbols, keysym);
    if (!keycode) {
        Logger::warning("No keycode found for keysym " + std::to_string(keysym));
        return 0;
    }
    
    uint8_t code = *keycode;
    free(keycode);
    
    return code;
}

} // namespace Keyboard
} // namespace X
