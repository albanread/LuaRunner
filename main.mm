//
// main_refactored.mm
// LuaRunner2 - SuperTerminal Lua Runtime Application (Refactored with BaseRunner)
//
// A standalone application that executes Lua scripts using LuaJIT in a blocking/sequential
// style on a background thread. The Lua program runs its main loop while
// the render thread updates the display at 60 FPS.
//
// Usage: LuaRunner2 script.lua
//
// Script Style:
//   while true do
//     -- Update game state
//     -- Draw frame
//     wait_frame()  -- Wait for next frame
//   end
//

#import "LuaBaseRunner.h"
#include <iostream>
#include "LuaBindings.h"

extern "C" {
#include <lua.hpp>
}

using namespace SuperTerminal;

// Forward reference to access LuaBaseRunner from C function
static LuaBaseRunner* g_runnerInstance = nullptr;

// =============================================================================
// FBRunner3 Runtime Function Stubs (for compatibility with FBTBindings)
// =============================================================================

extern "C" {
    void fbrunner3_runtime_print_text(const char* text) {
        // Stub: In LuaRunner2, we just use st_text_put directly
        // This function is called from FBTBindings for PRINT support
        if (text) {
            printf("%s", text);
        }
    }

    void fbrunner3_runtime_print_newline() {
        // Stub: Print newline
        printf("\n");
    }

    void fbrunner3_runtime_set_cursor(int x, int y) {
        // Stub: Set cursor position
        // In LuaRunner2, this is handled by st_text_locate
    }

    bool fbrunner3_should_stop_script() {
        // Stub: Check if script should stop
        // In LuaRunner2, we use BaseRunner's cancellation mechanism
        if (g_runnerInstance) {
            return NO; // Don't stop for now
        }
        return false;
    }
}

// =============================================================================
// LuaRunner2 - Lua Runtime using LuaBaseRunner
// =============================================================================

@interface LuaRunner2App : LuaBaseRunner
@property (assign) BOOL scriptRunning;
@property (assign) std::string currentScriptContent;
@property (assign) std::string currentScriptName;
@property (strong) NSString* windowSize;
@end

@implementation LuaRunner2App {
    lua_State* _luaState;
    std::thread _scriptThread;
    std::atomic<bool> _shouldStopScript;
    pthread_t _currentScriptThread;
    std::atomic<bool> _scriptThreadActive;
    std::mutex _threadMutex;
    std::condition_variable _threadFinishedCV;
}

// =============================================================================
// Framework Initialization Override
// =============================================================================

- (BOOL)initializeFramework {
    DisplayConfig config;

    // Default to fullhd if not specified
    NSString* size = self.windowSize ? self.windowSize : @"fullhd";

    // Configure window size based on preset
    if ([size isEqualToString:@"small"]) {
        config.windowWidth = 640;
        config.windowHeight = 480;
        config.cellWidth = 8;
        config.cellHeight = 16;
    } else if ([size isEqualToString:@"medium"]) {
        config.windowWidth = 800;
        config.windowHeight = 600;
        config.cellWidth = 8;
        config.cellHeight = 16;
    } else if ([size isEqualToString:@"large"]) {
        config.windowWidth = 1280;
        config.windowHeight = 720;
        config.cellWidth = 16;
        config.cellHeight = 20;
    } else if ([size isEqualToString:@"fullhd"]) {
        config.windowWidth = 1920;
        config.windowHeight = 1080;
        config.cellWidth = 16;
        config.cellHeight = 32;
    } else {
        NSLog(@"[LuaRunner2] Unknown window size '%@', using fullhd", size);
        config.windowWidth = 1920;
        config.windowHeight = 1080;
        config.cellWidth = 16;
        config.cellHeight = 32;
    }

    config.fullscreen = false;
    config.vsync = true;
    config.targetFPS = 60.0f;
    config.windowTitle = self.runnerName.c_str();

    NSLog(@"[LuaRunner2] Configured window size: %dx%d (preset: %@)",
          config.windowWidth, config.windowHeight, size);

    return [self initializeFrameworkWithConfig:config];
}

// =============================================================================
// Menu Bar Setup Override
// =============================================================================

- (void)setupMenuBar {
    // Call parent implementation first
    [super setupMenuBar];

    // Get the main menu
    NSMenu* mainMenu = [[NSApplication sharedApplication] mainMenu];
    if (!mainMenu) {
        NSLog(@"[LuaRunner2] Warning: Main menu not available");
        return;
    }

    // Create Tools menu
    NSMenuItem* toolsMenuItem = [[NSMenuItem alloc] initWithTitle:@"Tools" action:nil keyEquivalent:@""];
    NSMenu* toolsMenu = [[NSMenu alloc] initWithTitle:@"Tools"];
    [toolsMenuItem setSubmenu:toolsMenu];

    // Add SPRED menu item
    NSMenuItem* spredItem = [[NSMenuItem alloc] initWithTitle:@"Open SPRED (Sprite Editor)"
                                                       action:@selector(openSPRED:)
                                                keyEquivalent:@""];
    [spredItem setTarget:self];
    [toolsMenu addItem:spredItem];

    // Add Tools menu before Window menu (if it exists) or at the end
    NSInteger windowMenuIndex = -1;
    for (NSInteger i = 0; i < [mainMenu numberOfItems]; i++) {
        NSMenuItem* item = [mainMenu itemAtIndex:i];
        if ([item.title isEqualToString:@"Window"]) {
            windowMenuIndex = i;
            break;
        }
    }

    if (windowMenuIndex >= 0) {
        [mainMenu insertItem:toolsMenuItem atIndex:windowMenuIndex];
    } else {
        [mainMenu addItem:toolsMenuItem];
    }

    NSLog(@"[LuaRunner2] Tools menu added with SPRED launcher");
}

- (void)openSPRED:(id)sender {
    NSLog(@"[LuaRunner2] Opening SPRED...");

    // Try to find SPRED.app in several locations
    NSArray* searchPaths = @[
        // 1. In Tools folder next to LuaTerminal.app (for packaged distribution)
        [[NSBundle mainBundle].bundlePath.stringByDeletingLastPathComponent stringByAppendingPathComponent:@"Tools/SPRED.app"],

        // 2. In the build directory (for development)
        @"/Users/oberon/FasterBasicGreen/spred/build/bin/SPRED.app",

        // 3. In Applications folder
        @"/Applications/SPRED.app",

        // 4. In user Applications folder
        [NSHomeDirectory() stringByAppendingPathComponent:@"Applications/SPRED.app"]
    ];

    NSString* spredPath = nil;
    for (NSString* path in searchPaths) {
        if ([[NSFileManager defaultManager] fileExistsAtPath:path]) {
            spredPath = path;
            NSLog(@"[LuaRunner2] Found SPRED at: %@", spredPath);
            break;
        }
    }

    if (!spredPath) {
        NSLog(@"[LuaRunner2] SPRED.app not found in any search location");

        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"SPRED Not Found"];
        [alert setInformativeText:@"Could not find SPRED.app. Please ensure it's installed in the Tools folder, Applications folder, or build directory."];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    }

    // Launch SPRED
    NSError* error = nil;
    NSURL* spredURL = [NSURL fileURLWithPath:spredPath];

    [[NSWorkspace sharedWorkspace] openApplicationAtURL:spredURL
                                          configuration:[NSWorkspaceOpenConfiguration configuration]
                                      completionHandler:^(NSRunningApplication *app, NSError *error) {
        if (error) {
            NSLog(@"[LuaRunner2] Failed to launch SPRED: %@", error.localizedDescription);

            dispatch_async(dispatch_get_main_queue(), ^{
                NSAlert* alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Failed to Launch SPRED"];
                [alert setInformativeText:[NSString stringWithFormat:@"Error: %@", error.localizedDescription]];
                [alert addButtonWithTitle:@"OK"];
                [alert runModal];
            });
        } else {
            NSLog(@"[LuaRunner2] SPRED launched successfully");
        }
    }];
}

// =============================================================================
// Runtime Initialization
// =============================================================================

- (BOOL)initializeRuntime {
    NSLog(@"[LuaRunner2] Initializing LuaJIT runtime...");

    // Initialize stop flag and thread state
    _shouldStopScript = false;
    _scriptThreadActive = false;
    _currentScriptThread = nullptr;

    // Initialize editor
    if (![self initializeEditor]) {
        NSLog(@"[LuaRunner2] WARNING: Failed to initialize editor");
    }

    // Set language for editor
    self.currentScriptLanguage = "lua";

    // Start in editor mode if no script path provided
    if (self.scriptPath.empty() || self.scriptPath == "untitled.lua") {
        NSLog(@"[LuaRunner2] Starting in editor mode");

        [self enterEditorMode];

        // Load welcome script
        NSLog(@"[LuaRunner2] Loading welcome script, textEditor=%p", self.textEditor.get());
        std::string welcome = "-- Welcome to LuaRunner2!\n"
                             "-- Write your Lua code here and press Cmd+R to run\n\n"
                             "while true do\n"
                             "  text_clear()\n"
                             "  text_put(0, 0, \"Hello from Lua!\", 0xFFFFFFFF, 0xFF000000)\n"
                             "  wait_frame()\n"
                             "end\n";

        if (self.textEditor) {
            self.textEditor->loadText(welcome);
            NSLog(@"[LuaRunner2] Welcome text loaded, buffer has %zu lines", self.textEditor->getLineCount());
            self.textEditor->setFilename("untitled");
        } else {
            NSLog(@"[LuaRunner2] ERROR: textEditor is NULL!");
        }
    }

    // Store runner instance for wait_frame callback
    g_runnerInstance = self;

    // Create Lua state
    _luaState = luaL_newstate();
    if (!_luaState) {
        NSLog(@"Failed to create Lua state");
        return NO;
    }

    // Load standard libraries
    luaL_openlibs(_luaState);

    // Register SuperTerminal API bindings
    LuaRunner2::registerBindings(_luaState);

    // Override wait_frame to use BaseRunner's frame synchronization and check for interruption
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        if (g_runnerInstance) {
            LuaRunner2App* runner = (LuaRunner2App*)g_runnerInstance;

            // Check if script should stop
            if (runner->_shouldStopScript) {
                luaL_error(L, "Script interrupted by user");
                return 0;
            }

            [g_runnerInstance waitForNextFrame];
        }
        return 0;
    });
    lua_setglobal(_luaState, "wait_frame");

    // Set up Lua debug hook to check for interruption every 100 instructions
    lua_sethook(_luaState, [](lua_State* L, lua_Debug* ar) {
        if (g_runnerInstance) {
            LuaRunner2App* runner = (LuaRunner2App*)g_runnerInstance;
            if (runner->_shouldStopScript) {
                luaL_error(L, "Script interrupted by user");
            }
        }
    }, LUA_MASKCOUNT, 100);

    // Add custom print function that logs to console
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        int nargs = lua_gettop(L);
        std::string message;
        for (int i = 1; i <= nargs; i++) {
            if (i > 1) message += "\t";
            if (lua_isstring(L, i)) {
                message += lua_tostring(L, i);
            } else if (lua_isnil(L, i)) {
                message += "nil";
            } else if (lua_isboolean(L, i)) {
                message += lua_toboolean(L, i) ? "true" : "false";
            } else {
                message += luaL_typename(L, i);
            }
        }
        NSLog(@"[Lua] %s", message.c_str());
        return 0;
    });
    lua_setglobal(_luaState, "print");

    NSLog(@"[LuaRunner2] LuaJIT runtime initialized (JIT: %s)", LUAJIT_VERSION);
    return YES;
}

// =============================================================================
// Script Loading and Execution
// =============================================================================

- (BOOL)loadAndExecuteScript {
    // If we have editor mode and no file, don't auto-execute
    if (self.textEditor && (self.scriptPath.empty() || self.scriptPath == "untitled.lua")) {
        NSLog(@"[LuaRunner2] Editor mode - waiting for user to run script");
        return YES;
    }

    NSString* scriptPath = [NSString stringWithUTF8String:self.scriptPath.c_str()];
    NSLog(@"[LuaRunner2] Loading Lua script: %@", scriptPath);

    // Check if file exists
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:scriptPath]) {
        NSLog(@"[LuaRunner2] ERROR: Script file not found: %@", scriptPath);
        return NO;
    }

    // Load the script file
    if (luaL_loadfile(_luaState, self.scriptPath.c_str()) != LUA_OK) {
        const char* error = lua_tostring(_luaState, -1);
        NSLog(@"[LuaRunner2] ERROR: Lua compile error: %s", error);
        [self showError:[NSString stringWithFormat:@"Lua compile error:\n%s", error]];
        lua_pop(_luaState, 1);
        return NO;
    }

    NSLog(@"[LuaRunner2] Lua script loaded successfully");
    NSLog(@"[LuaRunner2] Starting Lua script execution...");

    // Execute the loaded script
    if (lua_pcall(_luaState, 0, 0, 0) != LUA_OK) {
        const char* error = lua_tostring(_luaState, -1);
        NSLog(@"[LuaRunner2] ERROR: Lua runtime error: %s", error);
        [self showError:[NSString stringWithFormat:@"Lua runtime error:\n%s", error]];
        lua_pop(_luaState, 1);
        return NO;
    }

    NSLog(@"[LuaRunner2] Lua script execution completed");
    return YES;
}

// =============================================================================
// Script Execution from Editor
// =============================================================================

- (void)runScript {
    if (!self.textEditor) {
        [super runScript];
        return;
    }

    // Get script content from editor
    std::string scriptContent = self.textEditor->getText();
    if (scriptContent.empty()) {
        NSLog(@"[LuaRunner2] No script to run");
        return;
    }

    // Stop any currently running script first and wait for it to complete
    if (_scriptThreadActive) {
        NSLog(@"[LuaRunner2] Stopping previous script before starting new one...");
        [self stopScript];

        pthread_t threadToJoin = nullptr;

        // Wait for the previous thread to finish using condition variable (with timeout)
        {
            std::unique_lock<std::mutex> lock(_threadMutex);
            auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(5);

            while (_scriptThreadActive) {
                if (_threadFinishedCV.wait_until(lock, timeout) == std::cv_status::timeout) {
                    NSLog(@"[LuaRunner2] WARNING: Previous script thread did not terminate within timeout");
                    break;
                }
            }

            if (!_scriptThreadActive) {
                NSLog(@"[LuaRunner2] Previous script stopped successfully");
                threadToJoin = _currentScriptThread;
                _currentScriptThread = nullptr;
            }
        } // Release lock before joining

        // Join the thread OUTSIDE the lock to prevent deadlock
        if (threadToJoin != nullptr) {
            NSLog(@"[LuaRunner2] Joining previous script thread...");
            pthread_join(threadToJoin, nullptr);
            NSLog(@"[LuaRunner2] Thread cleanup complete");
        }
    }

    // Save current script if it has a name (do this before switching modes)
    if (!self.currentScriptName.empty() && self.currentScriptName != "untitled") {
        [self saveScript];
    }

    // === APPLE II STYLE: Clear screen and enter runtime mode FIRST ===
    // This ensures the display is ready before script execution begins
    NSLog(@"[LuaRunner2] Switching to runtime mode (Apple II style)...");
    [self enterRuntimeMode];
    NSLog(@"[LuaRunner2] Runtime mode active, starting script execution");

    // Now prepare script execution
    self.currentScriptContent = scriptContent;
    self.scriptRunning = YES;
    _shouldStopScript = false;

    // Start script execution in background thread with increased stack size
    // URES mode and deep Lua call stacks require more than the default 512KB
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // Set stack size to 4MB (default is 512KB on macOS)
    size_t stack_size = 4 * 1024 * 1024;  // 4 MB
    pthread_attr_setstacksize(&attr, stack_size);

    // Retain self for the thread (without ARC)
    [self retain];

    // Mark thread as active
    {
        std::lock_guard<std::mutex> lock(_threadMutex);
        _scriptThreadActive = true;
    }

    // Create thread with custom attributes (JOINABLE - do NOT detach)
    pthread_t thread;
    int result = pthread_create(&thread, &attr, [](void* arg) -> void* {
        @autoreleasepool {
            LuaRunner2App* app = (LuaRunner2App*)arg;
            [app executeScriptContent:app.currentScriptContent];

            // Mark thread as inactive when done and notify waiters
            // IMPORTANT: notify_all() must be called while holding the lock
            // to prevent lost wakeup race condition
            {
                std::lock_guard<std::mutex> lock(app->_threadMutex);
                app->_scriptThreadActive = false;
                app->_threadFinishedCV.notify_all();
            }

            [app release];  // Release the retained reference
        }
        return nullptr;
    }, (void*)self);

    if (result == 0) {
        std::lock_guard<std::mutex> lock(_threadMutex);
        _currentScriptThread = thread;
        // Do NOT detach - we need to join it on restart/cleanup
    } else {
        NSLog(@"[LuaRunner2] ERROR: Failed to create script thread: %d", result);
        _scriptThreadActive = false;
        [self release];  // Release on failure
    }

    pthread_attr_destroy(&attr);

    NSLog(@"[LuaRunner2] Script execution started");
}

- (void)executeScriptContent:(const std::string&)content {
    @autoreleasepool {
        // Reset stop flag
        _shouldStopScript = false;

        // Load the script content
        if (luaL_loadstring(_luaState, content.c_str()) != LUA_OK) {
            const char* error = lua_tostring(_luaState, -1);
            NSLog(@"[LuaRunner2] ERROR: Lua compile error: %s", error);

            // Sync GPU before cleanup on compile error
            @try {
                NSLog(@"[LuaRunner2] Syncing GPU after compile error...");
                st_gpu_sync();
                NSLog(@"[LuaRunner2] GPU sync complete");
            } @catch (NSException *e) {
                NSLog(@"[LuaRunner2] WARNING: GPU sync failed after compile error: %@", e);
            }

            dispatch_async(dispatch_get_main_queue(), ^{
                [self showError:[NSString stringWithFormat:@"Lua compile error:\n%s", error ? error : "(unknown error)"]];
                [self enterEditorMode];
            });
            lua_pop(_luaState, 1);
            self.scriptRunning = NO;
            return;
        }

        // Execute the script
        if (lua_pcall(_luaState, 0, 0, 0) != LUA_OK) {
            const char* error = lua_tostring(_luaState, -1);
            if (!error) error = "(unknown error)";
            std::string errorStr(error);

            // Check if it was interrupted by user
            if (errorStr.find("interrupted by user") != std::string::npos) {
                NSLog(@"[LuaRunner2] Script interrupted by user");

                // Sync GPU before cleanup on interruption
                @try {
                    NSLog(@"[LuaRunner2] Syncing GPU after interruption...");
                    st_gpu_sync();
                    NSLog(@"[LuaRunner2] GPU sync complete");
                } @catch (NSException *e) {
                    NSLog(@"[LuaRunner2] WARNING: GPU sync failed after interruption: %@", e);
                }

                dispatch_async(dispatch_get_main_queue(), ^{
                    [self enterEditorMode];
                });
            } else {
                NSLog(@"[LuaRunner2] ERROR: Lua runtime error: %s", error ? error : "(unknown error)");

                // Sync GPU before cleanup on error
                @try {
                    NSLog(@"[LuaRunner2] Syncing GPU after error...");
                    st_gpu_sync();
                    NSLog(@"[LuaRunner2] GPU sync complete");
                } @catch (NSException *e) {
                    NSLog(@"[LuaRunner2] WARNING: GPU sync failed after error: %@", e);
                }

                dispatch_async(dispatch_get_main_queue(), ^{
                    [self showError:[NSString stringWithFormat:@"Lua runtime error:\n%s", error ? error : "(unknown error)"]];
                    [self enterEditorMode];
                });
            }
            lua_pop(_luaState, 1);
            self.scriptRunning = NO;
            return;
        }

        NSLog(@"[LuaRunner2] Script execution completed");

        // Sync GPU to ensure all commands complete before thread exits
        @try {
            NSLog(@"[LuaRunner2] Syncing GPU after script completion...");
            st_gpu_sync();
            NSLog(@"[LuaRunner2] GPU sync complete");
        } @catch (NSException *e) {
            NSLog(@"[LuaRunner2] WARNING: GPU sync failed after completion: %@", e);
        }

        self.scriptRunning = NO;

        // Return to editor mode when script completes
        dispatch_async(dispatch_get_main_queue(), ^{
            [self enterEditorMode];
        });
    }
}

- (void)stopScript {
    if (self.scriptRunning) {
        NSLog(@"[LuaRunner2] Stopping script execution...");

        // Signal the script to stop via the atomic flag
        _shouldStopScript = true;
        self.scriptRunning = NO;

        // CRITICAL: Wait for script thread to actually finish before cleanup
        // This prevents race conditions where the thread is still accessing
        // video mode resources (especially URES buffers) when we try to clean them up
        pthread_t threadToJoin = nullptr;
        {
            std::unique_lock<std::mutex> lock(_threadMutex);
            auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(5);

            NSLog(@"[LuaRunner2] Waiting for script thread to finish...");
            while (_scriptThreadActive) {
                if (_threadFinishedCV.wait_until(lock, timeout) == std::cv_status::timeout) {
                    NSLog(@"[LuaRunner2] WARNING: Script thread did not terminate within timeout");
                    break;
                }
            }

            if (!_scriptThreadActive) {
                NSLog(@"[LuaRunner2] Script thread stopped successfully");
                threadToJoin = _currentScriptThread;
                _currentScriptThread = nullptr;
            }
        } // Release lock before joining

        // Join the thread OUTSIDE the lock to prevent deadlock
        if (threadToJoin != nullptr) {
            NSLog(@"[LuaRunner2] Joining script thread...");
            pthread_join(threadToJoin, nullptr);
            NSLog(@"[LuaRunner2] Thread join complete");
        }

        // Now it's safe to clean up resources - thread is fully stopped
        // Sync GPU to ensure all pending commands complete before cleanup
        @try {
            NSLog(@"[LuaRunner2] Syncing GPU before cleanup...");
            st_gpu_sync();
            NSLog(@"[LuaRunner2] GPU sync complete");
        } @catch (NSException *e) {
            NSLog(@"[LuaRunner2] WARNING: GPU sync failed before cleanup: %@", e);
        }

        // Clear all text display items to prevent stale GPU resource access
        NSLog(@"[LuaRunner2] Clearing text display items...");
        st_text_clear_displayed();
        NSLog(@"[LuaRunner2] Text display items cleared");

        // Reset display mode to text mode (mode 0)
        NSLog(@"[LuaRunner2] Resetting display mode to text mode...");
        try {
            st_mode(0);
            NSLog(@"[LuaRunner2] Display mode reset complete");
        } catch (const std::exception& e) {
            NSLog(@"[LuaRunner2] ERROR: C++ exception during mode reset: %s", e.what());
        } catch (...) {
            NSLog(@"[LuaRunner2] ERROR: Unknown exception during mode reset");
        }

        NSLog(@"[LuaRunner2] Stop signal sent to script");
    } else {
        NSLog(@"[LuaRunner2] No script currently running");
    }
}

// =============================================================================
// Syntax Highlighting
// =============================================================================

- (std::vector<uint32_t>)highlightLine:(const std::string&)line
                            lineNumber:(size_t)lineNumber {
    std::vector<uint32_t> colors;
    colors.reserve(line.length());

    // Lua syntax highlighting colors (RRGGBBAA format)
    const uint32_t COLOR_KEYWORD = 0xC678DDFF;    // Purple for keywords
    const uint32_t COLOR_STRING = 0x89CA78FF;     // Green for strings
    const uint32_t COLOR_COMMENT = 0x7F8C8DFF;    // Gray for comments
    const uint32_t COLOR_NUMBER = 0xE5C07BFF;     // Yellow for numbers
    const uint32_t COLOR_FUNCTION = 0x61AFEFFF;   // Blue for function calls
    const uint32_t COLOR_DEFAULT = 0xD4D4D4FF;    // Light gray default

    // Lua keywords
    static const std::vector<std::string> keywords = {
        "and", "break", "do", "else", "elseif", "end", "false", "for",
        "function", "if", "in", "local", "nil", "not", "or", "repeat",
        "return", "then", "true", "until", "while"
    };

    size_t i = 0;
    while (i < line.length()) {
        char ch = line[i];

        // Comments
        if (ch == '-' && i + 1 < line.length() && line[i + 1] == '-') {
            // Rest of line is comment
            while (i < line.length()) {
                colors.push_back(COLOR_COMMENT);
                i++;
            }
            break;
        }

        // Strings
        if (ch == '"' || ch == '\'') {
            char quote = ch;
            colors.push_back(COLOR_STRING);
            i++;
            while (i < line.length() && line[i] != quote) {
                colors.push_back(COLOR_STRING);
                if (line[i] == '\\' && i + 1 < line.length()) {
                    i++;
                    colors.push_back(COLOR_STRING);
                }
                i++;
            }
            if (i < line.length()) {
                colors.push_back(COLOR_STRING);
                i++;
            }
            continue;
        }

        // Numbers
        if (std::isdigit(ch) || (ch == '.' && i + 1 < line.length() && std::isdigit(line[i + 1]))) {
            while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.' ||
                   line[i] == 'x' || line[i] == 'X' ||
                   (line[i] >= 'a' && line[i] <= 'f') ||
                   (line[i] >= 'A' && line[i] <= 'F'))) {
                colors.push_back(COLOR_NUMBER);
                i++;
            }
            continue;
        }

        // Identifiers and keywords
        if (std::isalpha(ch) || ch == '_') {
            size_t start = i;
            while (i < line.length() && (std::isalnum(line[i]) || line[i] == '_')) {
                i++;
            }
            std::string word = line.substr(start, i - start);

            // Check if keyword
            bool isKeyword = false;
            for (const auto& kw : keywords) {
                if (word == kw) {
                    isKeyword = true;
                    break;
                }
            }

            // Check if function call (followed by '(')
            bool isFunction = false;
            size_t j = i;
            while (j < line.length() && std::isspace(line[j])) j++;
            if (j < line.length() && line[j] == '(') {
                isFunction = true;
            }

            uint32_t color = isKeyword ? COLOR_KEYWORD :
                           isFunction ? COLOR_FUNCTION :
                           COLOR_DEFAULT;

            for (size_t k = start; k < i; k++) {
                colors.push_back(color);
            }
            continue;
        }

        // Default color
        colors.push_back(COLOR_DEFAULT);
        i++;
    }

    return colors;
}

// =============================================================================
// Application Termination
// =============================================================================

- (void)applicationWillTerminate:(NSNotification*)notification {
    NSLog(@"[LuaRunner2] Application terminating - forcing immediate exit");
    // Don't call cleanup, don't wait for threads, just exit NOW
    _exit(0);
}

// =============================================================================
// Runtime Cleanup
// =============================================================================

- (void)cleanupRuntime {
    NSLog(@"[LuaRunner2] Cleaning up Lua runtime...");

    // Stop any running script
    self.scriptRunning = NO;

    // Shutdown editor
    [self shutdownEditor];

    // Clear global runner reference
    g_runnerInstance = nullptr;

    if (_luaState) {
        lua_close(_luaState);
        _luaState = nullptr;
    }

    NSLog(@"[LuaRunner2] Lua runtime cleaned up");

    // Force immediate clean exit to avoid any shutdown issues
    // Use _exit(0) instead of exit(0) to bypass all cleanup including thread checks
    _exit(0);
}

// =============================================================================
// Frame Loop Override
// =============================================================================

- (void)onFrameTick {
    [super onFrameTick];

    // LuaBaseRunner now handles update and rendering in its onFrameTick
    // Editor mode: LuaBaseRunner calls updateEditor and renders directly
    // Runtime mode: LuaBaseRunner calls displayManager->renderFrame()
}

@end

// =============================================================================
// Main Entry Point
// =============================================================================

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        // Parse command-line arguments
        std::string scriptPath = "untitled.lua";
        std::string windowSize = "fullhd";  // default: 1920x1080
        bool hasScript = false;

        // Parse arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];

            if (arg == "-h" || arg == "--help") {
                std::cerr << "\n";
                std::cerr << "Usage: LuaRunner2 [OPTIONS] [script.lua]\n";
                std::cerr << "\n";
                std::cerr << "Options:\n";
                std::cerr << "  -s, --size SIZE   Window size preset:\n";
                std::cerr << "                    small:  640x480  (80x30 grid)\n";
                std::cerr << "                    medium: 800x600  (100x37 grid) \n";
                std::cerr << "                    large:  1280x720 (120x36 grid)\n";
                std::cerr << "                    fullhd: 1920x1080 (120x33 grid) [default]\n";
                std::cerr << "  -h, --help        Show this help\n";
                std::cerr << "\n";
                std::cerr << "Keyboard Shortcuts:\n";
                std::cerr << "  Cmd+N: New script\n";
                std::cerr << "  Cmd+O: Open script from database\n";
                std::cerr << "  Cmd+S: Save script\n";
                std::cerr << "  Cmd+R: Run script\n";
                std::cerr << "  Cmd+.: Stop script\n";
                std::cerr << "  Cmd+F: Find text\n";
                std::cerr << "  Cmd+L: Go to line\n";
                std::cerr << "  Cmd+Q: Quit application\n";
                std::cerr << "\n";
                return 0;
            } else if (arg == "-s" || arg == "--size") {
                if (i + 1 < argc) {
                    windowSize = argv[++i];
                } else {
                    std::cerr << "Error: --size requires an argument\n";
                    return 1;
                }
            } else if (arg[0] != '-') {
                // It's the script path
                scriptPath = arg;
                hasScript = true;
            } else {
                std::cerr << "Unknown option: " << arg << "\n";
                std::cerr << "Use -h or --help for usage information\n";
                return 1;
            }
        }

        if (!hasScript) {
            std::cerr << "\n";
            std::cerr << "=================================================\n";
            std::cerr << "  SuperTerminal Lua Runtime\n";
            std::cerr << "  Powered by LuaJIT with Integrated Editor\n";
            std::cerr << "=================================================\n";
            std::cerr << "\n";
            std::cerr << "Starting in editor mode (window: " << windowSize << ")...\n";
            std::cerr << "\n";
            std::cerr << "Keyboard Shortcuts:\n";
            std::cerr << "  Cmd+N: New script\n";
            std::cerr << "  Cmd+O: Open script from database\n";
            std::cerr << "  Cmd+S: Save script\n";
            std::cerr << "  Cmd+R: Run script\n";
            std::cerr << "  Cmd+.: Stop script\n";
            std::cerr << "  Cmd+F: Find text\n";
            std::cerr << "  Cmd+L: Go to line\n";
            std::cerr << "  Cmd+Q: Quit application\n";
            std::cerr << "\n";
            std::cerr << "Available APIs:\n";
            std::cerr << "  Text: text_put, text_clear, text_putchar\n";
            std::cerr << "  Graphics: gfx_rect, gfx_circle, gfx_line, gfx_clear\n";
            std::cerr << "  Sprites: sprite_create, sprite_draw\n";
            std::cerr << "  Audio: synth_note, music_play\n";
            std::cerr << "  Input: key_pressed, mouse_position\n";
            std::cerr << "  Frame: wait_frame, frame_count, time\n";
            std::cerr << "  Utils: rgb, rgba, hsv\n";
            std::cerr << "\n";
        }

        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        LuaRunner2App* delegate = [[LuaRunner2App alloc] initWithScriptPath:scriptPath
                                                                  runnerName:"SuperTerminal"];

        // Configure window size
        delegate.windowSize = [NSString stringWithUTF8String:windowSize.c_str()];

        [app setDelegate:delegate];
        [app run];

        return 0;
    }
}
