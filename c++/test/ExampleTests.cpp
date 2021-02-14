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

// sscal
// -----------------------------------------

TEST(SSCAL, one) {
    uint32_t const numBuffers = 1;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 10;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 0,1,2,3,4,5,6,7,8,9 };
    char const shader[] = "../../../glsl/sscal.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { 1.0F }, // Push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[0]));
    for(uint32_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i,out[i]);
    }
}

TEST(SSCAL, two) {
    uint32_t const numBuffers = 1;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 10;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 0,1,2,3,4,5,6,7,8,9 };
    char const shader[] = "../../../glsl/sscal.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { 2.0F }, // Push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[0]));
    for(uint32_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2*i,out[i]);
    }
}

TEST(SSCAL, random) {
    srand((unsigned int)time(NULL));
    uint32_t const numBuffers = 1;
    uint32_t const numPushConstants = 1;

    for(uint32_t i=0;i<RAND_RUNS;++i) {

        uint32_t const size = MIN_SIZE + (rand() % uint32_t(MAX_SIZE - MIN_SIZE + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        for(uint32_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
        }

        float const pushConstant = float(rand())/float(RAND_MAX);

        char const shader[] = "../../../glsl/sscal.spv";

        ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new uint32_t[numBuffers]{ size }, // Buffer sizes
            data, // Buffer data
            std::array<std::variant<uint32_t,float>, numPushConstants> { pushConstant }, // Push constants
            new uint32_t[3]{ size,1,1 }, // Invocations
            new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );

        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[0]));
        for(uint32_t i = 0; i < size; ++i) {
            //std::cout << out[i] << std::endl;
            ASSERT_EQ(pushConstant*data[0][i],out[i]);
        }
    }
}

// saxpy
// -----------------------------------------

TEST(SAXPY, one) {
    uint32_t const numBuffers = 2;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 10;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 0,1,2,3,4,5,6,7,8,9 };
    data[1] = new float[size]{ 9,8,7,6,5,4,3,2,1,0 };

    char const shader[] = "../../../glsl/saxpy.spv";
    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        2, //  Number of buffers
        new uint32_t[numBuffers]{ size, size }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { 1.0F }, // Push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    for(uint32_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i+(size-i-1),out[i]);
    }
}

TEST(SAXPY, two) {
    uint32_t const numBuffers = 2;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 10;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 0,1,2,3,4,5,6,7,8,9 };
    data[1] = new float[size]{ 9,8,7,6,5,4,3,2,1,0 };

    char const shader[] = "../../../glsl/saxpy.spv";
    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        2, //  Number of buffers
        new uint32_t[numBuffers]{ size, size }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { 2.0F }, // Push constants
        new uint32_t[3]{ size,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    for(uint32_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2*i+(size-i-1),out[i]);
    }
}

TEST(SAXPY, random) {
    srand((unsigned int)time(NULL));

    uint32_t const numBuffers = 2;
    uint32_t const numPushConstants = 1;

    for(uint32_t i=0;i<RAND_RUNS;++i) {

        uint32_t const size = MIN_SIZE + (rand() % uint32_t(MAX_SIZE - MIN_SIZE + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[size];

        for(uint32_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
            data[1][j] = float(rand())/float(RAND_MAX);
        }

        float const pushConstant = float(rand())/float(RAND_MAX);

        char const shader[] = "../../../glsl/saxpy.spv";
        ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
            shader,
            2, //  Number of buffers
            new uint32_t[numBuffers]{ size, size }, // Buffer sizes
            data, // Buffer data
            std::array<std::variant<uint32_t,float>, numPushConstants> { pushConstant }, // Push constants
            new uint32_t[3]{ size,1,1 }, // Invocations
            new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );

        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
        for(uint32_t i = 0; i < size; ++i) {
            //std::cout << out[i] << std::endl;
            ASSERT_EQ(pushConstant*data[0][i]+data[1][i],out[i]);
        }
    }
}

// sdot_f
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SDOT_F, one) {
    uint32_t const numBuffers = 3;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 10;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 0,1,2,3,4,5,6,7,8,9 };
    data[1] = new float[size]{ 9,8,7,6,5,4,3,2,1,0 };
    data[2] = new float[1];

    char const shader[] = "../../../glsl/sdot_f.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size, size, 1 }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
        new uint32_t[3]{ 1,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
    std::cout << *out << std::endl;
    ASSERT_NEAR(*out,120.0,EPSILON);
    //assert(false);
}
// 2 subgroups worth (70)
TEST(SDOT_F, two) {
    uint32_t const numBuffers = 3;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 70;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9
    };
    data[1] = new float[size]{
        9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0
    };
    data[2] = new float[1];

    char const shader[] = "../../../glsl/sdot_f.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size, size, 1 }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
        new uint32_t[3]{ 1,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
    std::cout << *out << std::endl;
    ASSERT_NEAR(*out,840.0,EPSILON); // 120*7
    //assert(false);
}

// 2 workgroups worth (1050)
TEST(SDOT_F, three) {
    uint32_t const numBuffers = 3;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 1050;

    float** data = new float*[numBuffers];
    // 21 x 50
    data[0] = new float[size]{ 
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9
    };
    // 21 x 50
    data[1] = new float[size]{
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,
        9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0
    };
    data[2] = new float[1];

    char const shader[] = "../../../glsl/sdot_f.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size, size, 1 }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
        new uint32_t[3]{ 1,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
    std::cout << *out << std::endl;
    ASSERT_NEAR(*out,12600.0,EPSILON); // 120 * 5 * 21
    //assert(false);
}

TEST(SDOT_F, random) {
    srand((unsigned int)time(NULL));

    uint32_t const numBuffers = 3;
    uint32_t const numPushConstants = 1;

    for(uint32_t i=0;i<RAND_RUNS;++i) {
        uint32_t max = WORKGROUP_SIZE*WORKGROUP_SIZE;
        uint32_t min = WORKGROUP_SIZE+1;
        uint32_t size = min + (rand() % uint32_t(max - min + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[size];
        data[2] = new float[1];

        for(uint32_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
            data[1][j] = float(rand())/float(RAND_MAX);
        }

        char const shader[] = "../../../glsl/sdot_f.spv";
        ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new uint32_t[numBuffers]{ size, size, 1 }, // Buffer sizes
            data, // Buffer data
            std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
            new uint32_t[3]{ 1,1,1 }, // Invocations
            new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );
        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
        float sum = 0;
        for(uint32_t i = 0; i < size; ++i) {
            //std::cout << out[i] << std::endl;
            sum += data[0][i] * data[1][i];
        }
        std::cout << *out << std::endl;
        ASSERT_NEAR(*out,sum,EPSILON);
    }
}

// snrm2_f
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SNRM2_F, one) {
    uint32_t const numBuffers = 2;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 10;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 0,1,2,3,4,5,6,7,8,9 };
    data[1] = new float[1];

    char const shader[] = "../../../glsl/snrm2_f.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size, 1 }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
        new uint32_t[3]{ 1,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    std::cout << *out << std::endl;
    ASSERT_NEAR(*out,16.881943,EPSILON);
    //assert(false);
}

// 2 subgroups worth (70)
TEST(SNRM2_F, two) {
    uint32_t const numBuffers = 2;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 70;

    float** data = new float*[numBuffers];
    data[0] = new float[size]{ 
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9
    };
    data[1] = new float[1];

    char const shader[] = "../../../glsl/snrm2_f.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size, 1 }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
        new uint32_t[3]{ 1,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    std::cout << *out << std::endl;
    ASSERT_NEAR(*out,44.665422,EPSILON); // 285*7
    //assert(false);
}

// 2 workgroups worth (1050)
TEST(SNRM2_F, three) {
    uint32_t const numBuffers = 2;
    uint32_t const numPushConstants = 1;
    uint32_t const size = 1050;

    float** data = new float*[numBuffers];
    // 21 x 50
    data[0] = new float[size]{ 
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
        0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9
    };
    data[1] = new float[1];

    char const shader[] = "../../../glsl/snrm2_f.spv";

    ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
        shader,
        numBuffers, //  Number of buffers
        new uint32_t[numBuffers]{ size, 1 }, // Buffer sizes
        data, // Buffer data
        std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
        new uint32_t[3]{ 1,1,1 }, // Invocations
        new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    std::cout << *out << std::endl;
    ASSERT_NEAR(*out,172.988438,EPSILON); // 285 * 5 * 21
    //assert(false);
}

TEST(SNRM2_F, random) {
    srand((unsigned int)time(NULL));

    uint32_t const numBuffers = 2;
    uint32_t const numPushConstants = 1;

    for(uint32_t i=0;i<RAND_RUNS;++i) {
        uint32_t max = WORKGROUP_SIZE*WORKGROUP_SIZE;
        uint32_t min = WORKGROUP_SIZE+1;
        uint32_t size = min + (rand() % uint32_t(max - min + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[1];

        for(uint32_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
        }

        char const shader[] = "../../../glsl/snrm2_f.spv";

        ComputeApp<numPushConstants> app = ComputeApp<numPushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new uint32_t[numBuffers]{ size, 1 }, // Buffer sizes
            data, // Buffer data
            std::array<std::variant<uint32_t,float>, numPushConstants> { size }, // Push constants
            new uint32_t[3]{ 1,1,1 }, // Invocations
            new uint32_t[3]{ WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );
        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
        float sum = 0;
        for(uint32_t i = 0; i < size; ++i) {
            //std::cout << out[i] << std::endl;
            sum += data[0][i] * data[0][i];
        }
        std::cout << *out << std::endl;
        ASSERT_NEAR(*out,sqrt(sum),EPSILON);
    }
}