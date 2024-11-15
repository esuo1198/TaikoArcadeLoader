#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <string>

typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_HOOKS = 5
} LogLevel;

/**
* Logger Struct Used to Store Logging Preferences and State
*/
typedef struct {
    LogLevel logLevel;
    FILE* logFile;
} Logger;

/* Initializes a global Logger instance. */
void InitializeLogger(LogLevel level, bool logToFile);

/* Logs a message with file and line information, if the log level permits. */
void LogMessage(const char* codeFile, int codeLine, const char* message, LogLevel messageLevel);

/* Converts a string to a LogLevel type. */
LogLevel GetLogLevel(const std::string& logLevelStr);

/* Cleans up the logger, closing files if necessary. */
void CleanupLogger();

#endif /* LOGGER_H */