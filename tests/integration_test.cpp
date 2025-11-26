#include <gtest/gtest.h>
#include "app/application.hpp"
#include "core/debug-messenger.hpp"

// Integration test to verify the app starts without errors,
// no crashes, and content is rendered correctly
class AppStartupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Reset validation error counters before each test
        ankh::DebugMessenger::reset_counters();
    }
};

// Test that the application starts up without validation errors
TEST_F(AppStartupTest, StartsWithoutValidationErrors)
{
    ankh::Application app;
    
    // Run for a few frames to ensure initialization completes
    constexpr uint32_t kTestFrameCount = 10;
    bool success = app.run_frames(kTestFrameCount);
    
    // Verify no crashes occurred
    EXPECT_TRUE(success) << "Application crashed during startup";
    
    // Verify no Vulkan validation errors
    uint32_t error_count = ankh::DebugMessenger::error_count();
    EXPECT_EQ(error_count, 0) << "Vulkan validation errors detected: " << error_count;
}

// Test that content is rendered (frames are actually processed)
TEST_F(AppStartupTest, ContentIsRendered)
{
    ankh::Application app;
    
    // Run for multiple frames
    constexpr uint32_t kTestFrameCount = 10;
    bool success = app.run_frames(kTestFrameCount);
    
    EXPECT_TRUE(success) << "Application failed to render frames";
}

// Test that repeated startup/shutdown cycles work correctly
TEST_F(AppStartupTest, MultipleStartupShutdownCycles)
{
    for (int i = 0; i < 3; ++i)
    {
        ankh::DebugMessenger::reset_counters();
        
        ankh::Application app;
        bool success = app.run_frames(5);
        
        EXPECT_TRUE(success) << "Cycle " << i << " failed";
        EXPECT_EQ(ankh::DebugMessenger::error_count(), 0) 
            << "Validation errors in cycle " << i;
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
