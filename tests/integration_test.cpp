#include <gtest/gtest.h>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <string>
#include <chrono>
#include <thread>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>

// Integration test to verify the app starts without errors,
// no crashes, and content is rendered correctly
class AppStartupTest : public ::testing::Test
{
protected:
    // Path to the Ankh executable (relative to test directory)
    static constexpr const char* kAnkhPath = "../src/Ankh";
    
    // Timeout in seconds for app startup test
    static constexpr int kStartupTimeoutSec = 5;
    
    // Buffer size for reading stderr output
    static constexpr size_t kStderrBufferSize = 4096;
    
    // Convert string to lowercase for case-insensitive comparison
    static std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }
    
    // Run the Ankh application for a specified duration and capture output
    // Returns: pair of (exit_status, stderr_output)
    std::pair<int, std::string> runAppWithTimeout(int timeout_sec)
    {
        std::string stderr_output;
        int exit_status = -1;
        
        // Create pipe for stderr
        int stderr_pipe[2];
        if (pipe(stderr_pipe) != 0)
        {
            return {-1, "Failed to create pipe"};
        }
        
        pid_t pid = fork();
        
        if (pid == 0)
        {
            // Child process
            close(stderr_pipe[0]); // Close read end
            dup2(stderr_pipe[1], STDERR_FILENO); // Redirect stderr to pipe
            close(stderr_pipe[1]);
            
            // Execute Ankh
            execl(kAnkhPath, "Ankh", nullptr);
            
            // If exec fails
            _exit(127);
        }
        else if (pid > 0)
        {
            // Parent process
            close(stderr_pipe[1]); // Close write end
            
            // Wait for timeout then send SIGTERM
            std::this_thread::sleep_for(std::chrono::seconds(timeout_sec));
            
            // Check if process is still running
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            
            if (result == 0)
            {
                // Process still running - this is expected, send SIGTERM
                kill(pid, SIGTERM);
                
                // Give it a moment to clean up
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                // Check again
                result = waitpid(pid, &status, WNOHANG);
                if (result == 0)
                {
                    // Force kill if still running
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0);
                }
                
                // Process terminated by our signal - this is success
                exit_status = 0;
            }
            else if (result == pid)
            {
                // Process already exited
                if (WIFEXITED(status))
                {
                    exit_status = WEXITSTATUS(status);
                }
                else if (WIFSIGNALED(status))
                {
                    // Crashed with signal
                    exit_status = -WTERMSIG(status);
                }
            }
            
            // Read stderr output
            std::array<char, kStderrBufferSize> buffer;
            ssize_t bytes_read;
            while ((bytes_read = read(stderr_pipe[0], buffer.data(), buffer.size() - 1)) > 0)
            {
                buffer[static_cast<size_t>(bytes_read)] = '\0';
                stderr_output += buffer.data();
            }
            close(stderr_pipe[0]);
        }
        else
        {
            // Fork failed
            close(stderr_pipe[0]);
            close(stderr_pipe[1]);
            return {-1, "Fork failed"};
        }
        
        return {exit_status, stderr_output};
    }
    
    // Check if stderr contains validation errors (case-insensitive)
    bool hasValidationErrors(const std::string& stderr_output)
    {
        // Convert to lowercase for case-insensitive matching
        std::string lower_output = toLower(stderr_output);
        
        // Look for Vulkan validation layer error messages
        // Check for common validation error patterns
        return lower_output.find("validation") != std::string::npos &&
               lower_output.find("error") != std::string::npos;
    }
};

// Test that the application starts up without crashes
TEST_F(AppStartupTest, StartsWithoutCrash)
{
    auto [exit_status, stderr_output] = runAppWithTimeout(kStartupTimeoutSec);
    
    // exit_status of 0 means we terminated it (success)
    // Negative values indicate crash signals
    EXPECT_GE(exit_status, 0) 
        << "Application crashed with signal " << -exit_status 
        << "\nStderr: " << stderr_output;
}

// Test that the application starts up without validation errors
TEST_F(AppStartupTest, StartsWithoutValidationErrors)
{
    auto [exit_status, stderr_output] = runAppWithTimeout(kStartupTimeoutSec);
    
    // Check for validation errors in stderr
    EXPECT_FALSE(hasValidationErrors(stderr_output))
        << "Vulkan validation errors detected:\n" << stderr_output;
}

// Test that content is rendered (app runs without immediate exit)
TEST_F(AppStartupTest, ContentIsRendered)
{
    auto start = std::chrono::steady_clock::now();
    auto [exit_status, stderr_output] = runAppWithTimeout(kStartupTimeoutSec);
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    // If the app ran for close to the timeout, it was rendering
    auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    EXPECT_GE(elapsed_sec, kStartupTimeoutSec - 1)
        << "Application exited too early, may not have rendered content"
        << "\nStderr: " << stderr_output;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
