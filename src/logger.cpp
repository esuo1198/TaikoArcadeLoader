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
    if (loggerInstance == nullptr) {
        loggerInstance = (Logger *)malloc (sizeof (Logger));
        if (consoleHandle == 0) consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);
    }

    loggerInstance->logLevel = level;

    if (logToFile) {
        loggerInstance->logFile = fopen ("TaikoArcadeLoader.log", "w"); // Open in write mode
        if (!loggerInstance->logFile) LogMessage (LOG_LEVEL_WARN, "Failed to open TaikoArcadeLoader.log for writing.");
    } else loggerInstance->logFile = nullptr; // No file logging
}

void
LogMessageHandler (const char *function, const char *codeFile, int codeLine, LogLevel messageLevel, const std::string format, ...) {
    // Return if no logger or log level is too high
    if (loggerInstance == nullptr || messageLevel > loggerInstance->logLevel) return;

    // Lock for thread safety
    std::lock_guard<std::mutex> lock (logMutex);

    // Format the user-provided message
    va_list args;
    va_start (args, format);
    int requiredSize = vsnprintf (nullptr, 0, format.c_str (), args) + 1; // +1 for null terminator
    std::unique_ptr<char[]> buffer (new char[requiredSize]);              // Allocate buffer dynamically
    vsnprintf (buffer.get (), requiredSize, format.c_str (), args);       // Format the string
    std::string formattedMessage (buffer.get ());                         // Convert to std::string
    va_end (args);

    // Determine log type string
    std::string logType = GetLogLevelString (messageLevel);

    // Get current time and milliseconds
    SYSTEMTIME systemTime;
    GetSystemTime (&systemTime);
    time_t rawtime      = time (nullptr);
    struct tm *timeinfo = localtime (&rawtime);
    std::ostringstream timeStamp;
    timeStamp << std::put_time (timeinfo, "%Y/%m/%d %H:%M:%S") << "." << std::setw (3) << std::setfill ('0') << systemTime.wMilliseconds;

    // Construct the log message
    std::ostringstream logStream;
    logStream << function << " (" << codeFile << ":" << codeLine << "): " << formattedMessage;
    std::string logMessage = logStream.str ();

    // Print the log message
    std::cout << "[" << timeStamp.str () << "] ";                             // Timestamp
    SetConsoleTextAttribute (consoleHandle, GetLogLevelColor (messageLevel)); // Set Level color
    std::cout << logType;                                                     // Level
    SetConsoleTextAttribute (consoleHandle, 4 | 6 | 7 | 9 | 10 | 13);         // Reset console color
    std::cout << logMessage << std::endl;                                     // Log message
    std::cout.flush ();                                                       // Flush to ensure immediate writing

    if (loggerInstance->logFile) {
        fprintf (loggerInstance->logFile, "[%s] %s%s\n", timeStamp.str ().c_str (), logType.c_str (), logMessage.c_str ());
        fflush (loggerInstance->logFile); // Flush to ensure immediate writing
    }
}

void
LogMessageHandler (const char *function, const char *codeFile, int codeLine, LogLevel messageLevel, const std::wstring format, ...) {
    std::string utf8Message = ConvertWideToUtf8 (format); // Convert wide string to UTF-8

    va_list args;
    va_start (args, format);
    LogMessageHandler (function, codeFile, codeLine, messageLevel, utf8Message, args); // Delegate to the original handler
    va_end (args);
}

void
CleanupLogger () {
    if (loggerInstance != NULL) {
        if (loggerInstance->logFile) fclose (loggerInstance->logFile);
        free (loggerInstance);
        loggerInstance = NULL;
    }
}
