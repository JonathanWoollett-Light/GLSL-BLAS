#include <gtest/gtest.h>
#include "../Example.hpp"

// Random floats
#include <cstdlib>
#include <ctime>

const uint32_t RAND_RUNS = 1;

const uint32_t WORKGROUP_SIZE = 1024;

const uint32_t MAX_SIZE = 1000000;
const uint32_t MIN_SIZE = 100000;

// TEST(Sanity, one) { EXPECT_EQ(TwentyOne(),21); }


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
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
        false
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
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
        false
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
            new uint32_t[3]{ size,1,1 }, // Invocations
            new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
            false
        );

        // Checking
        float* out = ComputeApp::map(app.device,app.bufferMemories[0]);
        for(uint32_t i=0;i<size;++i) {
            EXPECT_EQ(out[i],push_constant*data[0][i]);
        }
    }
}

TEST(SAXPY, one) {
    uint32_t size = 10;
    float** data = new float*[2];
    data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
    data[1] = new float[size]{ 9,8,7,6,5,4,3,2,1,0 };

    char shader[] = "../../../glsl/saxpy.spv";
    ComputeApp app = ComputeApp(
        shader,
        new uint32_t[2]{ size, size }, // Buffer sizes
        2, //  Number of buffers
        data, // Buffer data
        new float[1]{ 1 }, // Push constants
        1, // Number of push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
        false
    );

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[1]);
    for(uint32_t i=0;i<size;++i) {
        EXPECT_EQ(out[i],data[0][i]+data[1][i]);
    }
}
TEST(SAXPY, two) {
    uint32_t size = 10;
    float** data = new float*[2];
    data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
    data[1] = new float[size]{ 9,8,7,6,5,4,3,2,1,0 };

    char shader[] = "../../../glsl/saxpy.spv";
    ComputeApp app = ComputeApp(
        shader,
        new uint32_t[2]{ size, size }, // Buffer sizes
        2, //  Number of buffers
        data, // Buffer data
        new float[1]{ 2 }, // Push constants
        1, // Number of push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
        false
    );

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[1]);
    for(uint32_t i=0;i<size;++i) {
        EXPECT_EQ(out[i],2*data[0][i]+data[1][i]);
    }
}
TEST(SAXPY, random) {
    srand((unsigned int)time(NULL));

    for(uint32_t i=0;i<RAND_RUNS;++i) {

        uint32_t size = MIN_SIZE + (rand() % uint32_t(MAX_SIZE - MIN_SIZE + 1));

        float** data = new float*[2];
        data[0] = new float[size];
        data[1] = new float[size];

        for(uint32_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
            data[1][j] = float(rand())/float(RAND_MAX);
        }

        float push_constant = float(rand())/float(RAND_MAX);

        char shader[] = "../../../glsl/saxpy.spv";
        ComputeApp app = ComputeApp(
            shader,
            new uint32_t[2]{ size, size }, // Buffer sizes
            2, //  Number of buffers
            data, // Buffer data
            new float[1]{ push_constant }, // Push constants
            1, // Number of push constants
            new uint32_t[3]{ size,1,1 }, // Invocations
            new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
            false
        );

        // Checking
        float* out = ComputeApp::map(app.device,app.bufferMemories[1]);

        for(uint32_t i=0;i<size;++i) {
            ASSERT_EQ(out[i],push_constant*data[0][i]+data[1][i]);
        }
    }
}

// My GPU does not support 'VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION' thus all tests of shaders with it are disabled.
TEST(SDOT, DISABLED_one) {
    uint32_t size = 10;
    float** data = new float*[2];
    data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
    data[1] = new float[size]{ 9,8,7,6,5,4,3,2,1,0 };
    data[2] = new float[1]{ 0 };

    char shader[] = "../../../glsl/sdot.spv";
    ComputeApp app = ComputeApp(
        shader,
        new uint32_t[3]{ size, size, 1 }, // Buffer sizes
        3, //  Number of buffers
        data, // Buffer data
        new float[0]{}, // Push constants
        0, // Number of push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
        true // Requires atomic float
    );

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[2]);
    float sum = 0;
    for(uint32_t i=0;i<size;++i) {
        sum += data[0][i]*data[1][i];
    }
    ASSERT_EQ(*out,sum);
}

TEST(SDOT_PARTIAL, one) {
    uint32_t size = 10;
    uint32_t workgroups = ceil(size / static_cast<float>(WORKGROUP_SIZE));

    float** data = new float*[2];
    data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
    data[1] = new float[size]{ 9,8,7,6,5,4,3,2,1,0 };
    data[2] = new float[workgroups];

    char shader[] = "../../../glsl/sdot_partial.spv";
    ComputeApp app = ComputeApp(
        shader,
        new uint32_t[3]{ size, size, workgroups }, // Buffer sizes
        3, //  Number of buffers
        data, // Buffer data
        new float[0]{}, // Push constants
        0, // Number of push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 }, // Workgroup sizes
        false // Requires atomic float
    );

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[2]);
    float sum = 0;
    for(uint32_t i=0;i<size;++i) {
        sum += data[0][i]*data[1][i];
    }
    ASSERT_EQ(*out,sum);
}
