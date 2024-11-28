#pragma once

#include "helpers.h"
#include <source_location>
#include <string_view>
#include <wincon.h>

#define STRING(x)  #x
#define XSTRING(x) STRING (x)

enum class LogLevel {
    NONE = 0,
#ifdef ERROR
#undef ERROR
    ERROR,
#endif
    WARN,
    INFO,
    DEBUG,
    HOOKS
};

/**
 * Logger Struct Used to Store Logging Preferences and State
 */
typedef struct {
    LogLevel logLevel;
    FILE *logFile;
} Logger;

/* Initializes a global Logger instance. */
void InitializeLogger (LogLevel level, bool logToFile);

void LogMessageHandler (const char *function, const char *codeFile, int codeLine, LogLevel messageLevel, const char *format, ...);
void LogMessageHandler (const char *function, const char *codeFile, int codeLine, LogLevel messageLevel, const wchar_t *format, ...);

/* *
 * Logs a message with file and line information, if the log level permits.
 *
 * Use template magic to replace macros and get proper typing support...
 */
template <typename... Args>
struct LogMessage {
    LogMessage (const LogLevel level, const std::string_view format, Args &&...ts,
                const std::source_location &loc = std::source_location::current ()) {
        LogMessageHandler (loc.function_name (), loc.file_name (), loc.line (), level, format.data (), std::forward<Args> (ts)...);
    }

    LogMessage (const LogLevel level, const std::wstring_view format, Args &&...ts,
                const std::source_location &loc = std::source_location::current ()) {
        LogMessageHandler (loc.function_name (), loc.file_name (), loc.line (), level, format.data (), std::forward<Args> (ts)...);
    }
};

/* *
 * Logs a message with file and line information, if the log level permits.
 */
template <>
struct LogMessage<void> {
    LogMessage (const LogLevel level, const std::string_view format, const std::source_location &loc = std::source_location::current ()) {
        LogMessageHandler (loc.function_name (), loc.file_name (), loc.line (), level, format.data ());
    }

    LogMessage (const LogLevel level, const std::wstring_view format, const std::source_location &loc = std::source_location::current ()) {
        LogMessageHandler (loc.function_name (), loc.file_name (), loc.line (), level, format.data ());
    }
};

LogMessage (LogLevel level, std::string_view format) -> LogMessage<void>;
LogMessage (LogLevel level, std::wstring_view format) -> LogMessage<void>;

template <typename... Args>
LogMessage (LogLevel level, std::string_view format, Args &&...ts) -> LogMessage<Args...>;
template <typename... Args>
LogMessage (LogLevel level, std::wstring_view format, Args &&...ts) -> LogMessage<Args...>;

/* Converts a string to a LogLevel type. */
inline LogLevel
GetLogLevel (const std::string &logLevelStr) {
    if (logLevelStr == "DEBUG") return LogLevel::DEBUG;
    else if (logLevelStr == "INFO") return LogLevel::INFO;
    else if (logLevelStr == "WARN") return LogLevel::WARN;
    else if (logLevelStr == "ERROR") return LogLevel::ERROR;
    else if (logLevelStr == "HOOKS") return LogLevel::HOOKS;
    return LogLevel::NONE;
}

/* Converts a LogLevel type to a string for logging. */
inline std::string
GetLogLevelString (LogLevel messageLevel) {
    switch (messageLevel) {
    case LogLevel::DEBUG: return "DEBUG: ";
    case LogLevel::INFO: return "INFO:  ";
    case LogLevel::WARN: return "WARN:  ";
    case LogLevel::ERROR: return "ERROR: ";
    case LogLevel::HOOKS: return "HOOKS: ";
    default: return "NONE: ";
    }
}

/* Converts a LogLevel type to an int for colors display in the console. */
inline int
GetLogLevelColor (const LogLevel messageLevel) {
    // Colors: https://i.sstatic.net/ZG625.png
    switch (messageLevel) {
    case LogLevel::DEBUG: return FOREGROUND_BLUE | FOREGROUND_INTENSITY;                  // Pale Blue
    case LogLevel::INFO: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;                  // Pale Green
    case LogLevel::WARN: return FOREGROUND_RED | FOREGROUND_GREEN;                        // Bright Yellow
    case LogLevel::ERROR: return FOREGROUND_RED;                                          // Bright RED
    case LogLevel::HOOKS: return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY; // Pale Purple
    default: return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    }
}

/* Cleans up the logger, closing files if necessary. */
void CleanupLogger ();
