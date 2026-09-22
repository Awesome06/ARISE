cmake_minimum_required(VERSION 3.20)
project(ai_agent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ── Dependencies ─────────────────────────────────────────────────────────────

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# ── whisper.cpp: prefer a local checkout, fall back to FetchContent ───────────
# This preserves your existing workflow on your dev machine without any changes.
# On CI (Docker) or a fresh machine, it will download whisper.cpp automatically.
set(LOCAL_WHISPER_DIR "/home/Aurelius/Documents/AdoVs/whisper.cpp")

if(EXISTS "${LOCAL_WHISPER_DIR}/CMakeLists.txt")
    message(STATUS "ARISE: using local whisper.cpp at ${LOCAL_WHISPER_DIR}")
    set(WHISPER_DIR "${LOCAL_WHISPER_DIR}")
    add_subdirectory(${WHISPER_DIR} ${CMAKE_BINARY_DIR}/whisper EXCLUDE_FROM_ALL)
    set(WHISPER_LIBS whisper ggml)
    set(WHISPER_INCLUDES
        ${WHISPER_DIR}/include
        ${WHISPER_DIR}/ggml/include
    )
else()
    message(STATUS "ARISE: local whisper.cpp not found — fetching from GitHub")
    FetchContent_Declare(
        whisper_cpp
        GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
        GIT_TAG        v1.7.1
        GIT_SHALLOW    TRUE
    )
    set(WHISPER_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(whisper_cpp)
    set(WHISPER_LIBS whisper)
    set(WHISPER_INCLUDES "")
endif()

# ── Core library (shared between the main binary and the test binary) ─────────
# Excludes main.cpp so tests can link against logic without a duplicate main().
add_library(arise_core STATIC
    src/daemon.cpp
    src/logger.cpp
    src/recorder.cpp
    src/transcriber.cpp
    src/llm.cpp
    src/executor.cpp
    src/tts.cpp
)

target_include_directories(arise_core PUBLIC
    src/
    ${WHISPER_INCLUDES}
)

target_link_libraries(arise_core PUBLIC
    ${WHISPER_LIBS}
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

target_compile_options(arise_core PRIVATE -Wall -Wextra -O2)
target_compile_definitions(arise_core PRIVATE _POSIX_C_SOURCE=200809L)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(ai-agent src/main.cpp)
target_link_libraries(ai-agent PRIVATE arise_core)
target_compile_options(ai-agent PRIVATE -Wall -Wextra -O2)

# ── Unit Tests (Google Test) ──────────────────────────────────────────────────
enable_testing()

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
add_subdirectory(tests)