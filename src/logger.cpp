/*
// Copyright (c) 2019 Onur Dundar
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "logger.h"
#include "helpers.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

static Logger *loggerInstance = NULL;
void *consoleHandle           = 0;
std::mutex logMutex; // Mutex for thread-safe logging

char timeStr[64];
time_t rawtime;
struct tm *timeinfo;
SYSTEMTIME systemTime;

void
InitializeLogger (LogLevel level, bool logToFile) {
    if (loggerInstance == NULL) {
        loggerInstance = (Logger *)malloc (sizeof (Logger));

        // Ensure that console handle is initialized
        if (consoleHandle == 0) consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);
    }

    loggerInstance->logLevel = level;

    if (logToFile) {
        loggerInstance->logFile = fopen ("TaikoArcadeLoader.log", "w"); // Open in write mode
        if (!loggerInstance->logFile) LogMessage (__FUNCTION__, __FILE__, __LINE__, "Failed to open log.txt for writing.", LOG_LEVEL_WARN);
    }
}

void
LogMessage (const char *function, const char *codeFile, int codeLine, const char *message, LogLevel messageLevel) {
    if (loggerInstance == nullptr || messageLevel > loggerInstance->logLevel) return;

    // Lock for thread safety
    std::lock_guard<std::mutex> lock (logMutex);

    // Get current time
    time_t rawtime;
    struct tm *timeinfo;
    char timeStr[20]; // Buffer for time string
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (timeStr, sizeof (timeStr), "%Y/%m/%d %H:%M:%S", timeinfo);

    // Get milliseconds
    SYSTEMTIME systemTime;
    GetSystemTime (&systemTime);
    int milliseconds = systemTime.wMilliseconds;

    const char *logType;
    switch (messageLevel) {
    case LOG_LEVEL_DEBUG: logType = "DEBUG: "; break;
    case LOG_LEVEL_INFO: logType = "INFO:  "; break;
    case LOG_LEVEL_WARN: logType = "WARN:  "; break;
    case LOG_LEVEL_ERROR: logType = "ERROR: "; break;
    case LOG_LEVEL_HOOKS: logType = "HOOKS: "; break;
    default: logType = "NONE: "; break;
    }

    // Construct the full log message
    char logMessage[512];
    snprintf (logMessage, sizeof (logMessage), "%s (%s:%d): %s", function, codeFile, codeLine, message);

    // Print to the console
    printf ("[%s.%03d] ", timeStr, milliseconds);

    // Set console color based on message level
    switch (messageLevel) {
    case LOG_LEVEL_DEBUG: SetConsoleTextAttribute (consoleHandle, FOREGROUND_BLUE); break;
    case LOG_LEVEL_INFO: SetConsoleTextAttribute (consoleHandle, FOREGROUND_GREEN); break;
    case LOG_LEVEL_WARN: SetConsoleTextAttribute (consoleHandle, 6); break;
    case LOG_LEVEL_ERROR: SetConsoleTextAttribute (consoleHandle, FOREGROUND_RED); break;
    case LOG_LEVEL_HOOKS: SetConsoleTextAttribute (consoleHandle, 5); break;
    default: break;
    }

    // Print the log type (level)
    printf ("%s", logType);

    // Reset the console text color to default
    SetConsoleTextAttribute (consoleHandle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | 6);

    // Print the actual log message
    printf ("%s\n", logMessage);

    // Flush the output immediately to prevent buffering issues
    fflush (stdout);

    // Write to file if open
    if (loggerInstance->logFile) {
        fprintf (loggerInstance->logFile, "[%s.%03d] %s%s\n", timeStr, milliseconds, logType, logMessage);
        fflush (loggerInstance->logFile);
    }
}

LogLevel
GetLogLevel (const std::string &logLevelStr) {
    if (logLevelStr == "DEBUG") return LOG_LEVEL_DEBUG;
    else if (logLevelStr == "INFO") return LOG_LEVEL_INFO;
    else if (logLevelStr == "WARN") return LOG_LEVEL_WARN;
    else if (logLevelStr == "ERROR") return LOG_LEVEL_ERROR;
    else if (logLevelStr == "HOOKS") return LOG_LEVEL_HOOKS;
    return LOG_LEVEL_NONE;
}

void
CleanupLogger () {
    if (loggerInstance != NULL) {
        if (loggerInstance->logFile) fclose (loggerInstance->logFile);
        free (loggerInstance);
        loggerInstance = NULL;
    }
}
