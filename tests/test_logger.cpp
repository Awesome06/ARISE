// test_logger.cpp
// -----------------------------------------------------------------------------
// Unit tests for the Logger module.
// This suite verifies that the Logger does not crash under normal usage,
// correctly prefixes log messages with severity levels, and writes them 
// reliably to a temporary log file without data corruption.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>

#include "logger.hpp"

// ── Helpers ───────────────────────────────────────────────────────────────────

// A simple helper function to scan a file for a specific substring.
// This is used to verify that our log messages actually made it to the disk.
static bool fileContains(const std::string& path, const std::string& needle) {
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(needle) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// ── Test Fixture ──────────────────────────────────────────────────────────────

// The LoggerTest fixture sets up a clean environment before EACH test runs,
// and cleans up after EACH test finishes. This ensures tests are isolated.
class LoggerTest : public ::testing::Test {
protected:
    // We use a temporary path in /tmp so we don't pollute the project directory
    const std::string tmpLog = "/tmp/arise_test_logger.log";

    // SetUp() is executed automatically before every single TEST_F in this suite.
    void SetUp() override {
        // Remove any leftover file from a previous test run to ensure a clean slate
        std::remove(tmpLog.c_str());
        // Re-initialize the logger to point to our fresh temporary file
        Logger::init(tmpLog);
    }

    // TearDown() is executed automatically after every single TEST_F finishes.
    void TearDown() override {
        // Clean up the temporary file so we leave no trace behind on the system
        std::remove(tmpLog.c_str());
    }
};

// ── Tests ─────────────────────────────────────────────────────────────────────

// Tests that a standard INFO level message successfully reaches the file.
TEST_F(LoggerTest, InfoWritesToFile) {
    Logger::info("test info message");
    EXPECT_TRUE(fileContains(tmpLog, "test info message"));
}

// Tests that a WARNING level message successfully reaches the file.
TEST_F(LoggerTest, WarnWritesToFile) {
    Logger::warn("test warn message");
    EXPECT_TRUE(fileContains(tmpLog, "test warn message"));
}

// Tests that an ERROR level message successfully reaches the file.
TEST_F(LoggerTest, ErrorWritesToFile) {
    Logger::error("test error message");
    EXPECT_TRUE(fileContains(tmpLog, "test error message"));
}

// Verifies that the logger automatically prepends the [INFO] tag.
TEST_F(LoggerTest, LogLevelPrefixIsPresent) {
    Logger::info("level-prefix-check");
    EXPECT_TRUE(fileContains(tmpLog, "[INFO]"));
}

// Verifies that the logger automatically prepends the [WARN] tag.
TEST_F(LoggerTest, WarnLevelPrefixIsPresent) {
    Logger::warn("warn-prefix-check");
    EXPECT_TRUE(fileContains(tmpLog, "[WARN]"));
}

// Verifies that the logger automatically prepends the [ERROR] tag.
TEST_F(LoggerTest, ErrorLevelPrefixIsPresent) {
    Logger::error("error-prefix-check");
    EXPECT_TRUE(fileContains(tmpLog, "[ERROR]"));
}

// Edge Case: Passing an empty string should be handled gracefully, not crash.
TEST_F(LoggerTest, EmptyMessageDoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(Logger::info(""));
    EXPECT_NO_FATAL_FAILURE(Logger::warn(""));
    EXPECT_NO_FATAL_FAILURE(Logger::error(""));
}

// Verifies that writing multiple times in a row doesn't overwrite previous logs.
TEST_F(LoggerTest, MultipleWritesDontCorruptFile) {
    Logger::info("first");
    Logger::warn("second");
    Logger::error("third");
    
    // We expect all three lines to exist in the same file simultaneously.
    EXPECT_TRUE(fileContains(tmpLog, "first"));
    EXPECT_TRUE(fileContains(tmpLog, "second"));
    EXPECT_TRUE(fileContains(tmpLog, "third"));
}
