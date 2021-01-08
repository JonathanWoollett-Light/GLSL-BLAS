#include <gtest/gtest.h>
#include "../Example.hpp"

// Random floats
#include <cstdlib>
#include <ctime>
const uint32_t RAND_RUNS = 10;
const uint32_t MAX_SIZE = 1000000;
const uint32_t MIN_SIZE = 100000;

TEST(Sanity, one) {
    EXPECT_EQ(TwentyOne(),21);
    //EXPECT_EQ(true,false);
}


TEST(SSCAL, one) {
    uint32_t size = 10;
    float** data = new float*[1];
    data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
    char shader[] = "../../../glsl/sscal.spv";
    ComputeApp app = ComputeApp(
        shader,
        new uint32_t[1]{ size }, // Buffer sizes
        1, //  Number of buffers
        data, // Buffer data
        new float[1]{ 1 }, // Push constants
        1, // Number of push constants
        new int[3]{ 10,1,1 }, // Invocations
        new int[3]{ 1024,1,1 } // Workgroup sizes
    );

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[0]);
    for(uint32_t i=0;i<size;++i) {
        EXPECT_EQ(out[i],data[0][i]);
    }
}
TEST(SSCAL, two) {
    uint32_t size = 10;
    float** data = new float*[1];
    data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
    char shader[] = "../../../glsl/sscal.spv";
    ComputeApp app = ComputeApp(
        shader,
        new uint32_t[1]{ size }, // Buffer sizes
        1, //  Number of buffers
        data, // Buffer data
        new float[1]{ 2 }, // Push constants
        1, // Number of push constants
        new int[3]{ 10,1,1 }, // Invocations
        new int[3]{ 1024,1,1 } // Workgroup sizes
    );

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[0]);
    for(uint32_t i=0;i<size;++i) {
        EXPECT_EQ(out[i],2*data[0][i]);
    }
}
TEST(SSCAL, random) {
    srand((unsigned int)time(NULL));

    for(uint32_t i=0;i<RAND_RUNS;++i) {

        uint32_t size = MIN_SIZE + (rand() % uint32_t(MAX_SIZE - MIN_SIZE + 1));

        float** data = new float*[1];
        data[0] = new float[size];
        for(uint32_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
        }
        float push_constant = float(rand())/float(RAND_MAX);
        char shader[] = "../../../glsl/sscal.spv";
        ComputeApp app = ComputeApp(
            shader,
            new uint32_t[1]{ size }, // Buffer sizes
            1, //  Number of buffers
            data, // Buffer data
            new float[1]{ push_constant }, // Push constants
            1, // Number of push constants
            new int[3]{ static_cast<int>(size),1,1 }, // Invocations
            new int[3]{ 1024,1,1 } // Workgroup sizes
        );

        // Checking
        float* out = ComputeApp::map(app.device,app.bufferMemories[0]);
        for(uint32_t i=0;i<size;++i) {
            EXPECT_EQ(out[i],push_constant*data[0][i]);
        }
    }
    
}