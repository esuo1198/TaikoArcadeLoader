#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <string>

typedef enum { LOG_LEVEL_NONE = 0, LOG_LEVEL_ERROR = 1, LOG_LEVEL_WARN = 2, LOG_LEVEL_INFO = 3, LOG_LEVEL_DEBUG = 4, LOG_LEVEL_HOOKS = 5 } LogLevel;

/**
 * Logger Struct Used to Store Logging Preferences and State
 */
typedef struct {
    LogLevel logLevel;
    FILE *logFile;
} Logger;

/* Initializes a global Logger instance. */
void InitializeLogger (LogLevel level, bool logToFile);

/* Logs a message with file and line information, if the log level permits. */
#define LogMessage(level, format, ...) LogMessageHandler (__FUNCTION__, __FILE__, __LINE__, level, format, ##__VA_ARGS__)
void LogMessageHandler (const char *function, const char *codeFile, int codeLine, LogLevel messageLevel, const std::string format, ...);
void LogMessageHandler (const char *function, const char *codeFile, int codeLine, LogLevel messageLevel, const std::wstring format, ...);

/* Converts a string to a LogLevel type. */
LogLevel
GetLogLevel (const std::string &logLevelStr) {
    if (logLevelStr == "DEBUG") return LOG_LEVEL_DEBUG;
    else if (logLevelStr == "INFO") return LOG_LEVEL_INFO;
    else if (logLevelStr == "WARN") return LOG_LEVEL_WARN;
    else if (logLevelStr == "ERROR") return LOG_LEVEL_ERROR;
    else if (logLevelStr == "HOOKS") return LOG_LEVEL_HOOKS;
    return LOG_LEVEL_NONE;
}

/* Converts a LogLevel type to a string for logging. */
std::string
GetLogLevelString (LogLevel messageLevel) {
    switch (messageLevel) {
    case LOG_LEVEL_DEBUG: return "DEBUG: ";
    case LOG_LEVEL_INFO: return "INFO:  ";
    case LOG_LEVEL_WARN: return "WARN:  ";
    case LOG_LEVEL_ERROR: return "ERROR: ";
    case LOG_LEVEL_HOOKS: return "HOOKS: ";
    default: return "NONE: ";
    }
}

int
GetLogLevelColor (LogLevel messageLevel) {
    // Colors: https://i.sstatic.net/ZG625.png
    switch (messageLevel) {
    case LOG_LEVEL_DEBUG: return 9;  // Pale Blue
    case LOG_LEVEL_INFO: return 10;  // Pale Green
    case LOG_LEVEL_WARN: return 6;   // Bright Yellow
    case LOG_LEVEL_ERROR: return 4;  // Bright RED
    case LOG_LEVEL_HOOKS: return 13; // Pale Purple
    default: return 7;
    }
}

/* Cleans up the logger, closing files if necessary. */
void CleanupLogger ();

#endif /* LOGGER_H */