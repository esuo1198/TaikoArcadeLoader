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

static Logger *loggerInstance = nullptr;
void *consoleHandle           = nullptr;
std::mutex logMutex; // Mutex for thread-safe logging

char timeStr[64];
time_t rawTime;
tm *timeInfo;
SYSTEMTIME systemTime;

/*static std::unique_ptr<plog::RollingFileAppender<plog::TxtFormatter>> fileAppender;
static std::unique_ptr<plog::ConsoleAppender<plog::TxtFormatter>> consoleAppender;*/

void
InitializeLogger (const LogLevel level, const bool logToFile) {
    if (loggerInstance == nullptr) {
        loggerInstance = static_cast<Logger *> (malloc (sizeof (Logger)));
        if (consoleHandle == nullptr) consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);
    }

    loggerInstance->logLevel = level;

    if (logToFile) {
        loggerInstance->logFile = fopen ("TaikoArcadeLoader.log", "w"); // Open in write mode
        if (!loggerInstance->logFile) LogMessage (LogLevel::WARN, std::string ("Failed to open TaikoArcadeLoader.log for writing."));
    } else loggerInstance->logFile = nullptr; // No file logging

    /*consoleAppender = std::make_unique<plog::ConsoleAppender<plog::TxtFormatter>> ();
    auto& init =  plog::init (plog::verbose, consoleAppender.get ());
    if (logToFile) {
        fileAppender = std::make_unique<plog::RollingFileAppender<plog::TxtFormatter>> ("TaikoArcadeLoader.log", 1000000, 3);
        init.addAppender (fileAppender.get ());
    }*/
}

void
LogMessageHandler (const char *function, const char *codeFile, int codeLine, LogLevel messageLevel, const char *format, ...) {
    // Return if no logger or log level is too high
    if (loggerInstance == nullptr || messageLevel > loggerInstance->logLevel) return;

    // Lock for thread safety
    std::lock_guard lock (logMutex);

    // Format the user-provided message
    va_list args;
    va_start (args, format);
    int requiredSize = vsnprintf (nullptr, 0, format, args) + 1; // +1 for null terminator
    std::unique_ptr<char[]> buffer (new char[requiredSize]);     // Allocate buffer dynamically
    vsnprintf (buffer.get (), requiredSize, format, args);       // Format the string
    std::string formattedMessage (buffer.get ());                // Convert to std::string
    va_end (args);

    // Determine log type string
    std::string logType = GetLogLevelString (messageLevel);

    // Get current time and milliseconds
    SYSTEMTIME systemTime;
    GetSystemTime (&systemTime);
    time_t rawTime = time (nullptr);
    tm *timeInfo   = localtime (&rawTime);
    std::ostringstream timeStamp;
    timeStamp << std::put_time (timeInfo, "%Y/%m/%d %H:%M:%S") << "." << std::setw (3) << std::setfill ('0') << systemTime.wMilliseconds;

    // Construct the log message
    std::ostringstream logStream;
    logStream << function << " (" << codeFile << ":" << codeLine << "): " << formattedMessage;
    std::string logMessage = logStream.str ();

    // Print the log message
    std::cout << "[" << timeStamp.str () << "] ";                                                                        // Timestamp
    SetConsoleTextAttribute (consoleHandle, GetLogLevelColor (messageLevel));                                            // Set Level color
    std::cout << logType;                                                                                                // Level
    SetConsoleTextAttribute (consoleHandle, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Reset console color
    std::cout << logMessage << std::endl;                                                                                // Log message
    std::cout.flush (); // Flush to ensure immediate writing

    if (loggerInstance->logFile != nullptr) {
        fprintf (loggerInstance->logFile, "[%s] %s%s\n", timeStamp.str ().c_str (), logType.c_str (), logMessage.c_str ());
        fflush (loggerInstance->logFile); // Flush to ensure immediate writing
    }
}

void
LogMessageHandler (const char *function, const char *codeFile, const int codeLine, const LogLevel messageLevel, const wchar_t *format, ...) {
    const std::string utf8Message = ConvertWideToUtf8 (format); // Convert wide string to UTF-8

    va_list args;
    va_start (args, format);
    LogMessageHandler (function, codeFile, codeLine, messageLevel, utf8Message.c_str (), args); // Delegate to the original handler
    va_end (args);
}

void
CleanupLogger () {
    if (loggerInstance != nullptr) {
        if (loggerInstance->logFile) fclose (loggerInstance->logFile);
        free (loggerInstance);
        loggerInstance = nullptr;
    }
}
