#include <gtest/gtest.h>
#include "../Example.hpp"

// Random floats
#include <cstdlib>
#include <ctime>

#include <chrono> // Time tests

const size_t RAND_RUNS = 1;

const size_t WORKGROUP_SIZE = 1024;

const size_t MAX_SIZE = 1000000;
const size_t MIN_SIZE = 100000;

// TODO This seems extremely large, am I doing something wrong?
//  Also maybe use percentage difference instead.
const float EPSILON = 0.1F;

// sscal
// -----------------------------------------

TEST(SSCAL, one) {
    size_t const numBuffers = 1;
    size_t const numPushConstants = 1;
    size_t const size = 10;


    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9}
    );
    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {1.0F};

    char const shader[] = "../../../glsl/sscal.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[0]));
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i,out[i]);
    }
}

TEST(SSCAL, two) {
    size_t const numBuffers = 1;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9}
    );
    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {2.0F};
    char const shader[] = "../../../glsl/sscal.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[0]));
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2*i,out[i]);
    }
}
/*
TEST(SSCAL, random) {
    srand((unsigned int)time(NULL));

    size_t const numBuffers = 1;
    size_t const numPushConstants = 1;
    // static std::array<std::variant<size_t,float>,numPushConstants> pushConstants;

    for(size_t i=0;i<RAND_RUNS;++i) {

        size_t const size = MIN_SIZE + (rand() % size_t(MAX_SIZE - MIN_SIZE + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        for(size_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
        }

        float const alpha = float(rand())/float(RAND_MAX);
        // pushConstants[0] = alpha;
        // static constexpr std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = { float(rand())/float(RAND_MAX) };

        char const shader[] = "../../../glsl/sscal.spv";

        ComputeApp<numPushConstants,pushConstants> app = ComputeApp<numPushConstants,pushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new size_t[numBuffers]{ size }, // Buffer sizes
            data, // Buffer data
            std::array<size_t,3> { size,1,1 }, // Invocations
            std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );

        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[0]));
        for(size_t j = 0; j < size; ++j) {
            //std::cout << out[i] << std::endl;
            ASSERT_EQ(alpha*data[0][j],out[j]);
        }
    }
}
*/

// saxpy
// -----------------------------------------

TEST(SAXPY, one) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,size>{ 9,8,7,6,5,4,3,2,1,0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {1.0F};

    char const shader[] = "../../../glsl/saxpy.spv";
    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i+(size-i-1),out[i]);
    }
}

TEST(SAXPY, two) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,size>{ 9,8,7,6,5,4,3,2,1,0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {2.0F};

    char const shader[] = "../../../glsl/saxpy.spv";
    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2*i+(size-i-1),out[i]);
    }
}

/*
TEST(SAXPY, random) {
    srand((unsigned int)time(NULL));

    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    static std::array<std::variant<size_t,float>,numPushConstants> pushConstants;

    for(size_t i=0;i<RAND_RUNS;++i) {

        size_t const size = MIN_SIZE + (rand() % size_t(MAX_SIZE - MIN_SIZE + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[size];

        for(size_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
            data[1][j] = float(rand())/float(RAND_MAX);
        }

        float const alpha = float(rand())/float(RAND_MAX);
        pushConstants[0] = alpha;

        char const shader[] = "../../../glsl/saxpy.spv";
        ComputeApp<numPushConstants,pushConstants> app = ComputeApp<numPushConstants,pushConstants>(
            shader,
            2, //  Number of buffers
            new size_t[numBuffers]{ size, size }, // Buffer sizes
            data, // Buffer data
            std::array<size_t,3> { size,1,1 }, // Invocations
            std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );

        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
        for(size_t j = 0; j < size; ++j) {
            //std::cout << out[i] << std::endl;
            ASSERT_EQ(alpha*data[0][j]+data[1][j],out[j]);
        }
    }
}
*/

// sdot_f
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SDOT_F, one) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,size>{ 9,8,7,6,5,4,3,2,1,0 },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/sdot_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,120.0,EPSILON);
}
// 2 subgroups worth (70)
TEST(SDOT_F, two) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<float,size>{ 
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        },
        std::array<float,size> {
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0
        },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/sdot_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,840.0,EPSILON); // 120*7
}

// 2 workgroups worth (1050)
TEST(SDOT_F, three) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 1;
    size_t const size = 1050;

    auto data = std::make_tuple(
        std::array<float,size> { 
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
        },
        std::array<float,size> {
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
        },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/sdot_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,12600.0,EPSILON); // 120 * 5 * 21
}

/*
TEST(SDOT_F, random) {
    srand((unsigned int)time(NULL));

    size_t const numBuffers = 3;
    size_t const numPushConstants = 1;
    static std::array<std::variant<size_t,float>,numPushConstants> pushConstants;

    for(size_t i=0;i<RAND_RUNS;++i) {
        size_t const max = WORKGROUP_SIZE*WORKGROUP_SIZE;
        size_t const min = WORKGROUP_SIZE+1;
        size_t const size = min + (rand() % size_t(max - min + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[size];
        data[2] = new float[1];

        for(size_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
            data[1][j] = float(rand())/float(RAND_MAX);
        }

        pushConstants[0] = size;

        char const shader[] = "../../../glsl/sdot_f.spv";
        ComputeApp<numPushConstants,pushConstants> app = ComputeApp<numPushConstants,pushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new size_t[numBuffers]{ size, size, 1 }, // Buffer sizes
            data, // Buffer data
            std::array<size_t,3> { 1,1,1 }, // Invocations
            std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );
        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[2]));
        float sum = 0;
        for(size_t j = 0; j < size; ++j) {
            //std::cout << out[i] << std::endl;
            sum += data[0][j] * data[1][j];
        }
        // std::cout << *out << std::endl;
        ASSERT_NEAR(*out,sum,EPSILON);
    }
}
*/

// snrm2_f
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SNRM2_F, one) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/snrm2_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,16.881943,EPSILON);
}

// 2 subgroups worth (70)
TEST(SNRM2_F, two) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<float,size> { 
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/snrm2_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,44.665422,EPSILON); // 285*7
}

// 2 workgroups worth (1050)
TEST(SNRM2_F, three) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 1050;

    auto data = std::make_tuple(
        std::array<float,size> { 
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
        },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/snrm2_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,172.988438,EPSILON); // 285 * 5 * 21
}

/*
TEST(SNRM2_F, random) {
    srand((unsigned int)time(NULL));

    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    static std::array<std::variant<size_t,float>,numPushConstants> pushConstants;

    for(size_t i=0;i<RAND_RUNS;++i) {
        size_t const max = WORKGROUP_SIZE*WORKGROUP_SIZE;
        size_t const min = WORKGROUP_SIZE+1;
        size_t const size = min + (rand() % size_t(max - min + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[1];

        for(size_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
        }

        pushConstants[0] = size;

        char const shader[] = "../../../glsl/snrm2_f.spv";

        ComputeApp<numPushConstants,pushConstants> app = ComputeApp<numPushConstants,pushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new size_t[numBuffers]{ size, 1 }, // Buffer sizes
            data, // Buffer data
            std::array<size_t,3> { 1,1,1 }, // Invocations
            std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );
        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
        float sum = 0;
        for(size_t j = 0; j < size; ++j) {
            //std::cout << out[i] << std::endl;
            sum += data[0][j] * data[0][j];
        }
        // std::cout << *out << std::endl;
        ASSERT_NEAR(*out,sqrt(sum),EPSILON);
    }
}
*/

// sasum
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SASUM_F, one) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/sasum_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,45.0,EPSILON);
}

// 2 subgroups worth (70)
TEST(SASUM_F, two) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<float,size> { 
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/sasum_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,315,EPSILON); // 45*7
}

// 2 workgroups worth (1050)
TEST(SASUM_F, three) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 1050;

    auto data = std::make_tuple(
        std::array<float,size> { 
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
        },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/sasum_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,4725.0,EPSILON); // 45 * 5 * 21
}

/*
TEST(SASUM_F, random) {
    srand((unsigned int)time(NULL));

    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    static std::array<std::variant<size_t,float>,numPushConstants> pushConstants;

    for(size_t i=0;i<RAND_RUNS;++i) {
        size_t const max = WORKGROUP_SIZE*WORKGROUP_SIZE;
        size_t const min = WORKGROUP_SIZE+1;
        size_t const size = min + (rand() % size_t(max - min + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[1];

        for(size_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
        }

        pushConstants[0] = size;

        char const shader[] = "../../../glsl/sasum_f.spv";

        ComputeApp<numPushConstants,pushConstants> app = ComputeApp<numPushConstants,pushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new size_t[numBuffers]{ size, 1 }, // Buffer sizes
            data, // Buffer data
            std::array<size_t,3> { 1,1,1 }, // Invocations
            std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );
        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
        float sum = 0;
        for(size_t j = 0; j < size; ++j) {
            //std::cout << out[i] << std::endl;
            sum += abs(data[0][j]);
        }
        // std::cout << *out << std::endl;
        ASSERT_NEAR(*out,sum,EPSILON);
    }
}
*/

// isamax
// -----------------------------------------

// 1 subgroup worth (10)
TEST(ISAMAX_F, one) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/isamax_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    size_t* out = static_cast<size_t*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_EQ(*out,9);
}

// 2 subgroups worth (70)
TEST(ISAMAX_F, two) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<float,size> { 
            0,1,2,3,4,5,6,7,8,9,
            10,11,12,13,14,15,16,17,18,19,
            20,21,22,23,24,25,26,27,28,29,
            30,31,32,33,34,35,36,37,38,39,
            40,41,42,43,44,45,46,47,48,49,
            50,51,52,53,54,55,56,57,58,59,
            40,41,42,43,44,45,46,47,48,49
        },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = {size};

    char const shader[] = "../../../glsl/isamax_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    size_t* out = static_cast<size_t*>(Utility::map(app.device,app.bufferMemory[1]));
    // std::cout << *out << std::endl;
    ASSERT_EQ(*out,59);
}

/*
TEST(ISAMAX_F, random) {
    srand((unsigned int)time(NULL));

    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    static std::array<std::variant<size_t,float>,numPushConstants> pushConstants;

    for(size_t i=0;i<RAND_RUNS;++i) {
        size_t max = WORKGROUP_SIZE*WORKGROUP_SIZE;
        size_t min = WORKGROUP_SIZE+1;
        size_t size = min + (rand() % size_t(max - min + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[1];

        for(size_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
        }

        pushConstants[0] = size;

        char const shader[] = "../../../glsl/isamax_f.spv";

        ComputeApp<numPushConstants,pushConstants> app = ComputeApp<numPushConstants,pushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new size_t[numBuffers]{ size, 1 }, // Buffer sizes
            data, // Buffer data
            std::array<size_t,3> { 1,1,1 }, // Invocations
            std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );

        size_t* out = static_cast<size_t*>(Utility::map(app.device,app.bufferMemory[1]));
        float maxValue = abs(data[0][0]);
        size_t maxIndex = 0;
        for(size_t j = 1; j < size; ++j) {
            // std::cout << out[i] << std::endl;
            float absValue = abs(data[0][j]);
            if (absValue > maxValue) {
                maxValue = absValue;
                maxIndex = j;
            }
        }
        // std::cout << *out << std::endl;
        ASSERT_EQ(*out,maxIndex);
    }
}
*/

// sgemv
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SGEMV_F, one) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 3;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<float,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<float,size*size> {
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = { 1.0F, 1.0F, size };
    
    char const shader[] = "../../../glsl/sgemv_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float expected[size] = { 
        285.0F, 286.0F, 287.0F, 288.0F, 289.0F, 290.0F, 291.0F, 292.0F, 293.0F, 294.0F
    };
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}

// 1 subgroup worth (10) non-1 scalars
TEST(SGEMV_F, two) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 3;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<float,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<float,size*size> {
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        }
    );

    static std::array<std::variant<size_t,float>,numPushConstants> const pushConstants = { 0.9248F, 1.73F, size };
    
    char const shader[] = "../../../glsl/sgemv_f.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float expected[size] = { 
        263.568F, 265.298F, 267.028F, 268.758F, 270.488F, 272.218F, 273.948F, 275.678F, 277.408F, 279.138F
    };
    float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}

/*
TEST(SGEMV_F, random) {
    srand((unsigned int)time(NULL));
    size_t const numBuffers = 3;
    size_t const numPushConstants = 3;
    static std::array<std::variant<size_t,float>,numPushConstants> pushConstants;

    for(size_t i=0;i<RAND_RUNS;++i) {
        size_t const max = 10;//WORKGROUP_SIZE*WORKGROUP_SIZE;
        size_t const min = 5;//WORKGROUP_SIZE+1;
        size_t const size = min + (rand() % size_t(max - min + 1));

        float** data = new float*[numBuffers];
        data[0] = new float[size];
        data[1] = new float[size];
        data[2] = new float[size*size];

        for(size_t j=0;j<size;++j) {
            data[0][j] = float(rand())/float(RAND_MAX);
            data[1][j] = float(rand())/float(RAND_MAX);
            const size_t row = size*j;
            for(size_t k=0;k<size;++k) {
                data[2][row+k] = float(rand())/float(RAND_MAX);
            }
        }

        float const alpha = float(rand())/float(RAND_MAX);
        pushConstants[0] = alpha;
        float const beta = float(rand())/float(RAND_MAX);
        pushConstants[1] = beta;
        pushConstants[2] = size;

        char const shader[] = "../../../glsl/sgemv_f.spv";

        ComputeApp<numPushConstants,pushConstants> app = ComputeApp<numPushConstants,pushConstants>(
            shader,
            numBuffers, //  Number of buffers
            new size_t[numBuffers]{ size, size, size*size }, // Buffer sizes
            data, // Buffer data
            std::array<size_t,3> { 1,1,1 }, // Invocations
            std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
        );

        float* expected = new float[size];
        for(size_t j=0;j<size;++j) {
            float rSum = 0;
            const size_t row = size*j;
            for(size_t k=0;k<size;++k) {
                rSum += data[2][row+k] * data[0][k];
            }
            rSum *= alpha;
            rSum += beta * data[1][j];
            expected[j] = rSum;
        }

        float* out = static_cast<float*>(Utility::map(app.device,app.bufferMemory[1]));
        for(size_t j = 0; j < size; ++j) {
            // std::cout << out[j] << std::endl;
            ASSERT_NEAR(expected[j],out[j],EPSILON);
        }
    }
}
*/