#include <gtest/gtest.h>
#include "../Example.hpp"

// Random floats
#include <cstdlib>
#include <ctime>

#include <chrono> // Time tests

const uint32_t RAND_RUNS = 1;

const uint32_t WORKGROUP_SIZE = 1024;

const uint32_t MAX_SIZE = 1000000;
const uint32_t MIN_SIZE = 100000;

// TODO This seems extraordinarily large, am I doing something wrong?
//  Also maybe use percentage difference instead.
const float EPSILON = 0.1;

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
        false,
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
        false,
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
            false,
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
        false,
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
        false,
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
            false,
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
    float** data = new float*[3];
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
        true, // Requires atomic float
        false
    );

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[2]);
    float sum = 0;
    for(uint32_t i=0;i<size;++i) {
        sum += data[0][i]*data[1][i];
    }
    ASSERT_EQ(*out,sum);
}

// Constant values requiring 1 application of the partial extension shader
TEST(SDOT_PARTIAL, one) {
    uint32_t size = WORKGROUP_SIZE+1;
    uint32_t workgroups = ceil(size / static_cast<float>(WORKGROUP_SIZE));

    float** data = new float*[3];
    data[0] = new float[size];
    data[1] = new float[size];
    data[2] = new float[workgroups];

    for(uint32_t j=0;j<size;++j) {
        data[0][j] = 1;
        data[1][j] = 2;
    }

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
        false, // Requires atomic float
        true
    );

    // ComputeApp::print(app.device,app.bufferMemories[2],workgroups);
    // ASSERT_EQ(false,true);

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[2]);
    ASSERT_EQ(*out,size*2);
}
// Random values requiring 1 application of the partial extension shader
TEST(SDOT_PARTIAL, random_one) {
    srand((unsigned int)time(NULL));

    for(uint32_t i=0;i<RAND_RUNS;++i) {
        uint32_t max = WORKGROUP_SIZE*WORKGROUP_SIZE;
        uint32_t min = WORKGROUP_SIZE+1;
        uint32_t size = min + (rand() % uint32_t(max - min + 1));
        uint32_t workgroups = ceil(size / static_cast<float>(WORKGROUP_SIZE));

        float** data = new float*[3];
        data[0] = new float[size];
        data[1] = new float[size];
        data[2] = new float[workgroups];

        for(uint32_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
            data[1][j] = float(rand())/float(RAND_MAX);
        }

        //auto start = std::chrono::high_resolution_clock::now();
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
            false, // Requires atomic float
            true
        );
        auto gpu_duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-start);

        // ComputeApp::print(app.device,app.bufferMemories[2],workgroups);
        // ASSERT_EQ(false,true);

        // Checking
        float* out = ComputeApp::map(app.device,app.bufferMemories[2]);

        //start = std::chrono::high_resolution_clock::now();
        float sum = 0;
        for(uint32_t i=0;i<size;++i) {
            sum += data[0][i]*data[1][i];
        }

        // auto cpu_duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-start);
        // std::cout << "gpu: " << gpu_duration.count() << "cpu: " << cpu_duration.count() << std::endl;
        // ASSERT_LT(gpu_duration,cpu_duration);
        
        ASSERT_NEAR(*out,sum,EPSILON);
    }
}
// Constant values requiring 2 applications of the partial extension shader
TEST(SDOT_PARTIAL, two) {
    uint32_t size = WORKGROUP_SIZE*WORKGROUP_SIZE+1; //1024*1024+1
    uint32_t workgroups = ceil(size / static_cast<float>(WORKGROUP_SIZE));

    float** data = new float*[3];
    data[0] = new float[size];
    data[1] = new float[size];
    data[2] = new float[workgroups];

    for(uint32_t j=0;j<size;++j) {
        data[0][j] = 1;
        data[1][j] = 2;
    }

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
        false, // Requires atomic float
        true
    );

    // ComputeApp::print(app.device,app.bufferMemories[2],workgroups);
    // ASSERT_EQ(false,true);

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[2]);
    ASSERT_EQ(*out,size*2);
}
// Doesn't work, I beleive due to size
// Constant values requiring 3 applications of the partial extension shader
TEST(SDOT_PARTIAL, DISABLED_three) {
    uint32_t size = WORKGROUP_SIZE*WORKGROUP_SIZE*WORKGROUP_SIZE+1; //1024*1024+1
    uint32_t workgroups = ceil(size / static_cast<float>(WORKGROUP_SIZE));

    float** data = new float*[3];
    data[0] = new float[size];
    data[1] = new float[size];
    data[2] = new float[workgroups];

    for(uint32_t j=0;j<size;++j) {
        data[0][j] = 1;
        data[1][j] = 2;
    }

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
        false, // Requires atomic float
        true
    );

    // ComputeApp::print(app.device,app.bufferMemories[2],workgroups);
    // ASSERT_EQ(false,true);

    // Checking
    float* out = ComputeApp::map(app.device,app.bufferMemories[2]);
    ASSERT_EQ(*out,size*2);
}