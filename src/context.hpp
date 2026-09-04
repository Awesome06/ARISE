#pragma once
#include <string>

struct SystemContext {
    std::string activeWindow;
    std::string activeApp;
    std::string clipboard;
    std::string screenText;
    std::string recentNotifications;
};

class Context {
public:
    static SystemContext capture();

    // True for terminal emulators, whose window title is almost always the
    // running shell command rather than anything about the window. Exposed so
    // Executor's list_windows suppresses the same titles Context::capture()
    // already drops — otherwise ARIA reads your shell history out loud.
    static bool isTerminalApp(const std::string& app);
};