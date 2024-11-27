cmake_minimum_required(VERSION 3.25)

project(TaikoArcadeLoader VERSION 1.0.0 LANGUAGES C CXX)

# Set C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set warning level
if(MSVC)
    add_compile_options(/W3)
    # Set the source file encoding to UTF-8
    add_compile_options(/utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add project definitions
add_definitions(-DNOMINMAX -DLTC_NO_PROTOTYPES -D_CRT_SECURE_NO_WARNINGS)

# Include FetchContent module
include(FetchContent)

# Fetch tomlc99 (no built-in CMake support)
FetchContent_Declare(
    tomlc99
    GIT_REPOSITORY https://github.com/cktan/tomlc99.git
    GIT_TAG master
)
FetchContent_GetProperties(tomlc99)
if(NOT tomlc99_POPULATED)
    FetchContent_MakeAvailable(tomlc99)
    add_library(tomlc99 STATIC ${tomlc99_SOURCE_DIR}/toml.c)
    target_include_directories(tomlc99 PUBLIC ${tomlc99_SOURCE_DIR})
endif()

# Fetch stb (header-only library, no built-in CMake support)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_MakeAvailable(stb)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endif()

# Fetch zxing-cpp
FetchContent_Declare(
    zxing_cpp
    URL https://github.com/zxing-cpp/zxing-cpp/archive/refs/tags/v2.2.1.zip
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zxing_cpp)

# Fetch zlib
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG 51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf
)
FetchContent_MakeAvailable(zlib)

# Fetch libtomcrypt
FetchContent_Declare(
    libtomcrypt
    GIT_REPOSITORY https://github.com/libtom/libtomcrypt.git
    GIT_TAG 124e020437715b0d2647ed12632fa10e2cfe9234 # v1.18.2 does not have cmake
)
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(WITH_LTM OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libtomcrypt)

# Fetch pugixml
FetchContent_Declare(
    pugixml
    URL https://github.com/zeux/pugixml/archive/v1.14.tar.gz
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(PUGIXML_WCHAR_MODE ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(pugixml)

# Fetch safetyhook
FetchContent_Declare(
    safetyhook
    GIT_REPOSITORY https://github.com/cursey/safetyhook.git
    GIT_TAG v0.4.1
)
set(SAFETYHOOK_FETCH_ZYDIS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(safetyhook)

# Fetch SDL2
FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.5/SDL2-2.26.5.tar.gz
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)

# Fetch xxHash
FetchContent_Declare(
    xxhash
    URL https://github.com/Cyan4973/xxHash/archive/v0.8.2.tar.gz
    SOURCE_SUBDIR cmake_unofficial
)
set(XXH_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(XXH_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(XXH_BUILD_XXHSUM OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(xxhash)

# Fetch Minhook
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG d862245d98fecd56dd7b1ca2f2e5185b75ecc780
)
FetchContent_MakeAvailable(minhook)

# Source files
set(SOURCES
    src/dllmain.cpp
    src/helpers.cpp
    src/logger.cpp
    src/poll.cpp
    src/bnusio.cpp
    src/patches/amauth.cpp
    src/patches/dxgi.cpp
    src/patches/fpslimiter.cpp
    src/patches/audio.cpp
    src/patches/qr.cpp
    src/patches/layeredfs.cpp
    src/patches/testmode.cpp
    src/patches/versions/JPN00.cpp
    src/patches/versions/JPN08.cpp
    src/patches/versions/JPN39.cpp
    src/patches/versions/CHN00.cpp
)

# Create the library
add_library(bnusio SHARED ${SOURCES})

# Remove 'lib' prefix from the library name
set_target_properties(bnusio PROPERTIES PREFIX "")

# Include directories
target_include_directories(bnusio PRIVATE
    src
    ${tomlc99_SOURCE_DIR}
    ${SDL2_SOURCE_DIR}/include
    ${xxhash_SOURCE_DIR}
    ${zlib_SOURCE_DIR}
    ${libtomcrypt_SOURCE_DIR}/src/headers
    ${minhook_SOURCE_DIR}/include
)

# Compiler definitions
target_compile_definitions(bnusio PRIVATE
    _WIN32_WINNT=_WIN32_WINNT_WIN10
)

# Add link options
if(NOT MSVC)
    target_link_options(bnusio PRIVATE -Wl,--allow-multiple-definition)
endif()

# Link libraries
target_link_libraries(bnusio PRIVATE
    tomlc99
    SDL2-static
    xxhash
    zlibstatic
    libtomcrypt
    safetyhook
    ZXing::ZXing
    pugixml
    stb
    ws2_32
    ntdll
    minhook
)

# Define log path; used to make the file path relative in the log calls.
# Last character (-) to remove the trailing slash in the log path
add_compile_definitions("SOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/src-")

# Set runtime library to static
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Define the .def file
set(DEF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/exports.def")

if (MSVC)
    # Add the .def file to the target properties
    set_target_properties(bnusio PROPERTIES LINK_FLAGS "/DEF:${DEF_FILE}")
endif()

add_custom_command(
    TARGET bnusio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bnusio.dll"
        "${CMAKE_SOURCE_DIR}/dist/bnusio.dll"
    COMMENT "Copying bnusio.dll to dist directory"
)

# Set a default target
add_custom_target(default ALL DEPENDS bnusio)