#include <gtest/gtest.h>
#include "../Example.hpp"

TEST(ExampleTests, BasicTest) {
    EXPECT_EQ(TwentyOne(),21);
}

TEST(ExampleTests, BasicGPUTest) {
    uint32_t size = 10;
    float** data = new float*[1];
    data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
    char shader[] = "../../../glsl/sscal.spv";
    ComputeApp app = ComputeApp(
        shader,
        new uint32_t[1]{ size }, // Buffer sizes
        1, //  Number of buffers
        data, // Buffer data
        new float[1]{ 7 }, // Push constants
        1, // Number of push constants
        new int[3]{ 10,1,1 }, // Invocations
        new int[3]{ 1024,1,1 } // Workgroup sizes
    );
    float* out = ComputeApp::map(app.device,app.bufferMemories[0]);
    for(uint32_t i=0;i<size;++i) {
        EXPECT_EQ(out[i],7*data[0][i]);
    }
}