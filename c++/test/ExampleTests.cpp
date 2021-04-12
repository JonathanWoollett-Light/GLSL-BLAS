#include <gtest/gtest.h>
#include "../Example.hpp"

// Random floats
#include <cstdlib>
#include <ctime>

#include <chrono> // Time tests

const size_t RAND_RUNS = 1;

const size_t WORKGROUP_SIZE = 1024;

constexpr size_t const MAX_SIZE = 10000;
constexpr size_t const MIN_SIZE = 1000;

constexpr size_t const LOWER_MAX_SIZE = 50;
constexpr size_t const LOWER_MIN_SIZE = 10;

// TODO This seems extremely large, am I doing something wrong?
//  Also maybe use percentage difference instead.
const float EPSILON = 0.1F;

// ----------------------------------------------------------------------------------
// sscal & dscal
// ----------------------------------------------------------------------------------

// -----------------------------------------
// sscal
// -----------------------------------------

TEST(SSCAL, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9}
    );
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 1.0F };

    char const shader[] = "../../../glsl/sscal.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[0]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i,out[i]);
    }
}

TEST(SSCAL, two) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9}
    );
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 2.0F };
    char const shader[] = "../../../glsl/sscal.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[0]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2*i,out[i]);
    }
}
// Random value test
TEST(SSCAL, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;

    constexpr size_t const size = MIN_SIZE + linearCongruentialGenerator(1) % (MAX_SIZE - MIN_SIZE + 1);

    std::array<float,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = float(rand())/float(RAND_MAX);
    }
    auto data = std::make_tuple(std::move(x));

    constexpr float const alpha = randToFloat(linearCongruentialGenerator(2));
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { alpha };

    char const shader[] = "../../../glsl/sscal.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[0]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_EQ(alpha*std::get<0>(data)[i],out[i]);
    }
}

// -----------------------------------------
// dscal
// -----------------------------------------

TEST(DSCAL, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 }
    );
    static std::array<std::variant<uint32_t, float, double>,numPushConstants> const pushConstants = { 1.00 };

    char const shader[] = "../../../glsl/dscal.spv";

    ComputeApp app = ComputeApp<numPushConstants, pushConstants, double, size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[0]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i,out[i]);
    }
}

TEST(DSCAL, two) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 }
    );
    static std::array<std::variant<uint32_t, float, double>,numPushConstants> const pushConstants = { 2.0 };
    char const shader[] = "../../../glsl/dscal.spv";

    ComputeApp app = ComputeApp<numPushConstants, pushConstants, double, size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[0]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2*i,out[i]);
    }
}
// Random value test
TEST(DSCAL, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;

    constexpr size_t const size = MIN_SIZE + linearCongruentialGenerator(1) % (MAX_SIZE - MIN_SIZE + 1);

    std::array<double,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = double(rand())/double(RAND_MAX);
    }
    auto data = std::make_tuple(std::move(x));

    constexpr double const alpha = randToFloat(linearCongruentialGenerator(2));
    static std::array<std::variant<uint32_t, float, double>,numPushConstants> const pushConstants = { alpha };

    char const shader[] = "../../../glsl/dscal.spv";

    ComputeApp app = ComputeApp<numPushConstants, pushConstants, double, size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[0]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_EQ(alpha*std::get<0>(data)[i],out[i]);
    }
}


// ----------------------------------------------------------------------------------
// saxpy & daxpy
// ----------------------------------------------------------------------------------

// -----------------------------------------
// saxpy
// -----------------------------------------

TEST(SAXPY, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,size>{ 9,8,7,6,5,4,3,2,1,0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {1.0F};

    char const shader[] = "../../../glsl/saxpy.spv";
    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i+(size-i-1),out[i]);
    }
}

TEST(SAXPY, two) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<float,size>{ 9,8,7,6,5,4,3,2,1,0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {2.0F};

    char const shader[] = "../../../glsl/saxpy.spv";
    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2*i+(size-i-1),out[i]);
    }
}
// Random value test
TEST(SAXPY, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(3) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<float,size> x;
    std::array<float,size> y;
    for(size_t i = 0; i < size; ++i) {
        x[i] = float(rand())/float(RAND_MAX);
        y[i] = float(rand())/float(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::move(y)
    );

    constexpr float const alpha = randToFloat(linearCongruentialGenerator(4));
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants { alpha };

    char const shader[] = "../../../glsl/saxpy.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_EQ(alpha*std::get<0>(data)[i]+std::get<1>(data)[i],out[i]);
    }
}

// -----------------------------------------
// daxpy
// -----------------------------------------

TEST(DAXPY, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,size>{ 9,8,7,6,5,4,3,2,1,0 }
    );

    static std::array<std::variant<uint32_t, float, double>,numPushConstants> const pushConstants = { 1.0 };

    char const shader[] = "../../../glsl/daxpy.spv";
    ComputeApp app = ComputeApp<numPushConstants, pushConstants, double, size, size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(i + (size - i - 1), out[i]);
    }
}

TEST(DAXPY, two) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,size>{ 9,8,7,6,5,4,3,2,1,0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 2.0 };

    char const shader[] = "../../../glsl/daxpy.spv";
    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        //std::cout << out[i] << std::endl;
        ASSERT_EQ(2 * i + (size - i - 1), out[i]);
    }
}
// Random value test
TEST(DAXPY, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(3) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<double,size> x;
    std::array<double,size> y;
    for(size_t i = 0; i < size; ++i) {
        x[i] = double(rand())/double(RAND_MAX);
        y[i] = double(rand())/double(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::move(y)
    );

    constexpr double const alpha = randToFloat(linearCongruentialGenerator(4));
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants { alpha };

    char const shader[] = "../../../glsl/daxpy.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { size,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_EQ(alpha * std::get<0>(data)[i] + std::get<1>(data)[i], out[i]);
    }
}

// ----------------------------------------------------------------------------------
// sdot & ddot
// ----------------------------------------------------------------------------------

// -----------------------------------------
// sdot
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SDOT, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<float,size>{ 9,8,7,6,5,4,3,2,1,0 },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sdot.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = Utility::map<float*>(app.device,app.bufferMemory[2]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,120.0,EPSILON);
}
// 2 subgroups worth (70)
TEST(SDOT, two) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sdot.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = Utility::map<float*>(app.device,app.bufferMemory[2]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,840.0,EPSILON); // 120*7
}
// 2 workgroups worth (1050)
TEST(SDOT, three) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sdot.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = Utility::map<float*>(app.device,app.bufferMemory[2]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,12600.0,EPSILON); // 120 * 5 * 21
}
// Random value test
TEST(SDOT, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(5) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<float,size> x;
    std::array<float,size> y;
    for(size_t i = 0; i < size; ++i) {
        x[i] = float(rand())/float(RAND_MAX);
        y[i] = float(rand())/float(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::move(y),
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sdot.spv";
    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[2]);
    float sum = 0;
    for(size_t i = 0; i < size; ++i) {
        sum += std::get<0>(data)[i] * std::get<1>(data)[i];
    }
    ASSERT_NEAR(*out,sum,2*EPSILON);
}

// -----------------------------------------
// ddot
// -----------------------------------------

// 1 subgroup worth (10)
TEST(DDOT, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,size>{ 9,8,7,6,5,4,3,2,1,0 },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/ddot.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    double* out = Utility::map<double*>(app.device,app.bufferMemory[2]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,120.0,EPSILON);
}
// 2 subgroups worth (70)
TEST(DDOT, two) {
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<double,size>{ 
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        },
        std::array<double,size> {
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0,
            9,8,7,6,5,4,3,2,1,0
        },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/ddot.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    double* out = Utility::map<double*>(app.device,app.bufferMemory[2]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,840.0,EPSILON); // 120*7
}
// 2 workgroups worth (1050)
TEST(DDOT, three) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 1;
    size_t const size = 1050;

    auto data = std::make_tuple(
        std::array<double,size> { 
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
        std::array<double,size> {
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
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/ddot.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    double* out = Utility::map<double*>(app.device,app.bufferMemory[2]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,12600.0,EPSILON); // 120 * 5 * 21
}
// Random value test
TEST(DDOT, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(5) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<double,size> x;
    std::array<double,size> y;
    for(size_t i = 0; i < size; ++i) {
        x[i] = double(rand())/double(RAND_MAX);
        y[i] = double(rand())/double(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::move(y),
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/ddot.spv";
    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[2]);
    double sum = 0;
    for(size_t i = 0; i < size; ++i) {
        sum += std::get<0>(data)[i] * std::get<1>(data)[i];
    }
    ASSERT_NEAR(*out,sum,2*EPSILON);
}

// ----------------------------------------------------------------------------------
// snrm2 & dnrm2
// ----------------------------------------------------------------------------------

// -----------------------------------------
// snrm2
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SNRM2, one) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/snrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    
    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    ASSERT_NEAR(*out,16.881943,EPSILON);
}
// 2 subgroups worth (70)
TEST(SNRM2, two) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/snrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    ASSERT_NEAR(*out,44.665422,EPSILON); // 285*7
}
// 2 workgroups worth (1050)
TEST(SNRM2, three) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/snrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    ASSERT_NEAR(*out,172.988438,EPSILON); // 285 * 5 * 21
}
// Random value test
TEST(SNRM2, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(6) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<float,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = float(rand())/float(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/snrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    float sum = 0;
    for(size_t i = 0; i < size; ++i) {
        sum += std::get<0>(data)[i] * std::get<0>(data)[i];
    }
    ASSERT_NEAR(*out,sqrt(sum),EPSILON);
}

// -----------------------------------------
// dnrm2
// -----------------------------------------

// 1 subgroup worth (10)
TEST(DNRM2, one) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dnrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    
    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    ASSERT_NEAR(*out,16.881943,EPSILON);
}
// 2 subgroups worth (70)
TEST(DNRM2, two) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<double,size> { 
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dnrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    ASSERT_NEAR(*out,44.665422,EPSILON); // 285*7
}
// 2 workgroups worth (1050)
TEST(DNRM2, three) {
    size_t const numBuffers = 2;
    size_t const numPushConstants = 1;
    size_t const size = 1050;

    auto data = std::make_tuple(
        std::array<double,size> { 
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
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dnrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    ASSERT_NEAR(*out,172.988438,EPSILON); // 285 * 5 * 21
}
// Random value test
TEST(DNRM2, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(6) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<double,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = double(rand())/double(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dnrm2.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    double sum = 0;
    for(size_t i = 0; i < size; ++i) {
        sum += std::get<0>(data)[i] * std::get<0>(data)[i];
    }
    ASSERT_NEAR(*out,sqrt(sum),EPSILON);
}

// ----------------------------------------------------------------------------------
// sasum & dasum
// ----------------------------------------------------------------------------------

// -----------------------------------------
// sasum
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SASUM, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{0,1,2,3,4,5,6,7,8,9},
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,45.0,EPSILON);
}
// 2 subgroups worth (70)
TEST(SASUM, two) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,315,EPSILON); // 45*7
}
// 2 workgroups worth (1050)
TEST(SASUM, three) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,4725.0,EPSILON); // 45 * 5 * 21
}
// Random value test
TEST(SASUM, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(7) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<float,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = float(rand())/float(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    float sum = 0;
    for(size_t i = 0; i < size; ++i) {
        sum += abs(std::get<0>(data)[i]);
    }
    ASSERT_NEAR(*out,sum,2*EPSILON);
}

// -----------------------------------------
// dasum
// -----------------------------------------

// 1 subgroup worth (10)
TEST(DASUM, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,45.0,EPSILON);
}
// 2 subgroups worth (70)
TEST(DASUM, two) {
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<double,size> { 
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9,
            0,1,2,3,4,5,6,7,8,9
        },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,315,EPSILON); // 45*7
}
// 2 workgroups worth (1050)
TEST(DASUM, three) {
    size_t const numPushConstants = 1;
    size_t const size = 1050;

    auto data = std::make_tuple(
        std::array<double,size> { 
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
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );
    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    // std::cout << *out << std::endl;
    ASSERT_NEAR(*out,4725.0,EPSILON); // 45 * 5 * 21
}
// Random value test
TEST(DASUM, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(7) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<double,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = double(rand()) / double(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dasum.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    double sum = 0;
    for(size_t i = 0; i < size; ++i) {
        sum += abs(std::get<0>(data)[i]);
    }
    ASSERT_NEAR(*out,sum,2*EPSILON);
}

// ----------------------------------------------------------------------------------
// isamax & idamax
// ----------------------------------------------------------------------------------

// -----------------------------------------
// isamax
// -----------------------------------------

// 1 subgroup worth (10)
TEST(ISAMAX, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<float,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<float,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/isamax.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    uint32_t* out = Utility::map<uint32_t*>(app.device,app.bufferMemory[1]);
    ASSERT_EQ(*out,9);
}
// 2 subgroups worth (70)
TEST(ISAMAX, two) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/isamax.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    uint32_t* out = Utility::map<uint32_t*>(app.device,app.bufferMemory[1]);
    ASSERT_EQ(*out,59);
}
// Random value test
TEST(ISAMAX, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(8) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<float,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = float(rand())/float(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::array<float,1>{ 0.0F }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/isamax.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    uint32_t* out = Utility::map<uint32_t*>(app.device,app.bufferMemory[1]);
    float maxValue = abs(std::get<0>(data)[0]);
    size_t maxIndex = 0;
    for(size_t i = 1; i < size; ++i) {
        float absValue = abs(std::get<0>(data)[i]);
        if (absValue > maxValue) {
            maxValue = absValue;
            maxIndex = i;
        }
    }
    // std::cout << maxIndex << " : " << *out << std::endl;
    // std::cout << maxValue << " : " << x[*out] << " : " << std::get<0>(data)[*out] << std::endl;
    // ASSERT_EQ(*out,static_cast<uint32_t>(maxIndex));
    ASSERT_NEAR(abs(std::get<0>(data)[*out]),maxValue,EPSILON);
}

// -----------------------------------------
// idamax
// -----------------------------------------

// 1 subgroup worth (10)
TEST(IDAMAX, one) {
    size_t const numPushConstants = 1;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size>{ 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/idamax.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    uint32_t* out = Utility::map<uint32_t*>(app.device,app.bufferMemory[1]);
    ASSERT_EQ(*out,9);
}
// 2 subgroups worth (70)
TEST(IDAMAX, two) {
    size_t const numPushConstants = 1;
    size_t const size = 70;

    auto data = std::make_tuple(
        std::array<double,size> { 
            0,1,2,3,4,5,6,7,8,9,
            10,11,12,13,14,15,16,17,18,19,
            20,21,22,23,24,25,26,27,28,29,
            30,31,32,33,34,35,36,37,38,39,
            40,41,42,43,44,45,46,47,48,49,
            50,51,52,53,54,55,56,57,58,59,
            40,41,42,43,44,45,46,47,48,49
        },
        std::array<double,1>{ 0 }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/idamax.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    uint32_t* out = Utility::map<uint32_t*>(app.device,app.bufferMemory[1]);
    ASSERT_EQ(*out,59);
}
// Random value test
TEST(IDAMAX, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 1;
    constexpr size_t const size = MIN_SIZE + (linearCongruentialGenerator(8) % (MAX_SIZE - MIN_SIZE + 1));

    std::array<double,size> x;
    for(size_t i = 0; i < size; ++i) {
        x[i] = double(rand())/double(RAND_MAX);
    }
    auto data = std::make_tuple(
        std::move(x),
        std::array<double,1>{ 0.0F }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/idamax.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,1>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    uint32_t* out = Utility::map<uint32_t*>(app.device,app.bufferMemory[1]);
    double maxValue = abs(std::get<0>(data)[0]);
    size_t maxIndex = 0;
    for(size_t i = 1; i < size; ++i) {
        double absValue = abs(std::get<0>(data)[i]);
        if (absValue > maxValue) {
            maxValue = absValue;
            maxIndex = i;
        }
    }
    // std::cout << maxIndex << " : " << *out << std::endl;
    // std::cout << maxValue << " : " << x[*out] << " : " << std::get<0>(data)[*out] << std::endl;
    // ASSERT_EQ(*out,static_cast<uint32_t>(maxIndex));
    ASSERT_NEAR(abs(std::get<0>(data)[*out]),maxValue,EPSILON);
}

// ----------------------------------------------------------------------------------
// sgemv & dgemv
// ----------------------------------------------------------------------------------

// -----------------------------------------
// sgemv
// -----------------------------------------

// 1 subgroup worth (10)
TEST(SGEMV, one) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 
        1.0F, 
        1.0F, 
        static_cast<uint32_t>(size)
    };
    
    char const shader[] = "../../../glsl/sgemv.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<float,size> const expected = { 
        285.0F, 286.0F, 287.0F, 288.0F, 289.0F, 290.0F, 291.0F, 292.0F, 293.0F, 294.0F
    };
    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}
// 1 subgroup worth (10) non-1 scalars
TEST(SGEMV, two) {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 
        0.9248F,
        1.73F,
        static_cast<uint32_t>(size)
    };
    
    char const shader[] = "../../../glsl/sgemv.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<float,size> const expected = { 
        263.568F, 265.298F, 267.028F, 268.758F, 270.488F, 272.218F, 273.948F, 275.678F, 277.408F, 279.138F
    };
    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}
// Random value test
TEST(SGEMV, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 3;
    constexpr size_t const size = LOWER_MIN_SIZE + (linearCongruentialGenerator(9) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));

    std::array<float,size> x;
    std::array<float,size> y;
    std::array<float,size*size> A;
    for(size_t i = 0; i < size; ++i) {
        x[i] = float(rand())/float(RAND_MAX);
        y[i] = float(rand())/float(RAND_MAX);
    }
    for(size_t i = 0; i < size * size; ++i) {
        A[i] = float(rand())/float(RAND_MAX);
    }

    auto data = std::make_tuple(
        std::move(x),
        std::move(y),
        std::move(A)
    );

    constexpr float const alpha = randToFloat(linearCongruentialGenerator(10));
    constexpr float const beta = randToFloat(linearCongruentialGenerator(11));
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        alpha,beta,static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/sgemv.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<float,size> expected;
    for(size_t i = 0; i < size; ++i) {
        float rSum = 0;
        const size_t row = size*i;
        for(size_t j = 0; j < size; ++j) {
            rSum += std::get<2>(data)[row+j] * std::get<0>(data)[j];
        }
        rSum *= alpha;
        rSum += beta * std::get<1>(data)[i];
        expected[i] = rSum;
    }

    float* out = Utility::map<float*>(app.device,app.bufferMemory[1]);
    for(size_t j = 0; j < size; ++j) {
        // std::cout << out[j] << std::endl;
        ASSERT_NEAR(expected[j],out[j],2*EPSILON);
    }
}

// -----------------------------------------
// dgemv
// -----------------------------------------

// 1 subgroup worth (10)
TEST(DGEMV, one) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 3;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,size*size> {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 
        1.0, 
        1.0, 
        static_cast<uint32_t>(size)
    };
    
    char const shader[] = "../../../glsl/dgemv.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<double,size> const expected = { 
        285.0, 286.0, 287.0, 288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0
    };
    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}
// 1 subgroup worth (10) non-1 scalars
TEST(DGEMV, two) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 3;
    size_t const size = 10;

    auto data = std::make_tuple(
        std::array<double,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,size> { 0,1,2,3,4,5,6,7,8,9 },
        std::array<double,size*size> {
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

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 
        0.9248,
        1.73,
        static_cast<uint32_t>(size)
    };
    
    char const shader[] = "../../../glsl/dgemv.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<double,size> const expected = { 
        263.568, 265.298, 267.028, 268.758, 270.488, 272.218, 273.948, 275.678, 277.408, 279.138
    };
    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    for(size_t i = 0; i < size; ++i) {
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}
// Random value test
TEST(DGEMV, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 3;
    constexpr size_t const size = LOWER_MIN_SIZE + (linearCongruentialGenerator(9) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));

    std::array<double,size> x;
    std::array<double,size> y;
    std::array<double,size*size> A;
    for(size_t i = 0; i < size; ++i) {
        x[i] = double(rand())/double(RAND_MAX);
        y[i] = double(rand())/double(RAND_MAX);
    }
    for(size_t i = 0; i < size * size; ++i) {
        A[i] = double(rand())/double(RAND_MAX);
    }

    auto data = std::make_tuple(
        std::move(x),
        std::move(y),
        std::move(A)
    );

    constexpr double const alpha = randToFloat(linearCongruentialGenerator(10));
    constexpr double const beta = randToFloat(linearCongruentialGenerator(11));
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        alpha,beta,static_cast<uint32_t>(size)
    };

    char const shader[] = "../../../glsl/dgemv.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,size,size,size*size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<double,size> expected;
    for(size_t i = 0; i < size; ++i) {
        double rSum = 0;
        const size_t row = size*i;
        for(size_t j = 0; j < size; ++j) {
            rSum += std::get<2>(data)[row+j] * std::get<0>(data)[j];
        }
        rSum *= alpha;
        rSum += beta * std::get<1>(data)[i];
        expected[i] = rSum;
    }

    double* out = Utility::map<double*>(app.device,app.bufferMemory[1]);
    for(size_t j = 0; j < size; ++j) {
        // std::cout << out[j] << std::endl;
        ASSERT_NEAR(expected[j],out[j],2*EPSILON);
    }
}

// ----------------------------------------------------------------------------------
// sgemm & dgemm
// ----------------------------------------------------------------------------------

// -----------------------------------------
// sgemm
// -----------------------------------------

// 1 subgroup worth (3,5,2)
TEST(SGEMM, one) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 5;
    size_t const m = 3;
    size_t const k = 5;
    size_t const n = 2;

    size_t const a_size = m * k;
    size_t const b_size = k * n;
    size_t const c_size = m * n;

    auto data = std::make_tuple(
        std::array<float,a_size> { 
            1,2,1,1,1,
            0,1,0,1,1,
            2,3,4,1,1 
        },
        std::array<float,b_size> { 
            2,5,
            6,7,
            1,8,
            1,1,
            1,1
        },
        std::array<float,c_size> {
            2,8,
            7,5,
            9,3
        }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 
        2.0F,
        3.0F,
        static_cast<uint32_t>(m),
        static_cast<uint32_t>(k),
        static_cast<uint32_t>(n)
    };
    
    char const shader[] = "../../../glsl/sgemm.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,a_size,b_size,c_size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<float,c_size> const expected = { 
        40,82,
        37,33,
        83,139
    };
    float* out = Utility::map<float*>(app.device,app.bufferMemory[2]);
    for(size_t i = 0; i < c_size; ++i) {
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}
// Random value test
TEST(SGEMM, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 5;

    constexpr size_t const m = LOWER_MIN_SIZE + (linearCongruentialGenerator(12) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));
    constexpr size_t const k = LOWER_MIN_SIZE + (linearCongruentialGenerator(13) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));
    constexpr size_t const n = LOWER_MIN_SIZE + (linearCongruentialGenerator(14) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));

    size_t const a_size = m * k;
    size_t const b_size = k * n;
    size_t const c_size = m * n;

    std::array<float,a_size> A;
    std::array<float,b_size> B;
    std::array<float,c_size> C;
    std::array<float,c_size> C_cpu; // cpu side verison of C which retains the original values
    for(size_t i = 0; i < a_size; ++i) {
        A[i] = float(rand())/float(RAND_MAX);
    }
    for(size_t i = 0; i < b_size; ++i) {
        B[i] = float(rand())/float(RAND_MAX);
    }
    for(size_t i = 0; i < c_size; ++i) {
        C[i] = float(rand())/float(RAND_MAX);
        C_cpu[i] = C[i];
    }

    auto data = std::make_tuple(
        std::move(A),
        std::move(B),
        std::move(C)
    );

    constexpr float const alpha = randToFloat(linearCongruentialGenerator(15));
    constexpr float const beta = randToFloat(linearCongruentialGenerator(16));
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        alpha,
        beta,
        static_cast<uint32_t>(m),
        static_cast<uint32_t>(k),
        static_cast<uint32_t>(n)
    };

    char const shader[] = "../../../glsl/sgemm.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,float,a_size,b_size,c_size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<float,c_size> expected;
    for(size_t m_index = 0; m_index < m; ++m_index) {
        const size_t A_row = k * m_index;
        for(size_t n_index = 0; n_index < n; ++n_index) {
            float temp = 0;
            for(size_t k_index = 0; k_index < k; ++k_index) {
                temp += std::get<0>(data)[A_row + k_index] * std::get<1>(data)[n * k_index + n_index];
                // std::cout << std::get<0>(data)[A_row + k_index] << '*' << std::get<1>(data)[n * k_index + n_index] << ' ';
            }
            // std::cout << temp << ' ';
            const size_t C_index = n * m_index + n_index;
            expected[C_index] = alpha * temp + beta * C_cpu[C_index];
            // std::cout << alpha << '*' << temp << '+' << beta << '*' << C_cpu[C_index] << '=' << expected[C_index] << std::endl;
        }
    }
    //assert(false);

    float* out = Utility::map<float*>(app.device,app.bufferMemory[2]);
    for(size_t i = 0; i < c_size; ++i) {
        ASSERT_NEAR(expected[i],out[i],2*EPSILON);
    }
}

// -----------------------------------------
// dgemm
// -----------------------------------------

// 1 subgroup worth (3,5,2)
TEST(DGEMM, one) {
    size_t const numBuffers = 3;
    size_t const numPushConstants = 5;
    size_t const m = 3;
    size_t const k = 5;
    size_t const n = 2;

    size_t const a_size = m * k;
    size_t const b_size = k * n;
    size_t const c_size = m * n;

    auto data = std::make_tuple(
        std::array<double,a_size> { 
            1,2,1,1,1,
            0,1,0,1,1,
            2,3,4,1,1 
        },
        std::array<double,b_size> { 
            2,5,
            6,7,
            1,8,
            1,1,
            1,1
        },
        std::array<double,c_size> {
            2,8,
            7,5,
            9,3
        }
    );

    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = { 
        2.0,
        3.0,
        static_cast<uint32_t>(m),
        static_cast<uint32_t>(k),
        static_cast<uint32_t>(n)
    };
    
    char const shader[] = "../../../glsl/dgemm.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,a_size,b_size,c_size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<double,c_size> const expected = { 
        40,82,
        37,33,
        83,139
    };
    double* out = Utility::map<double*>(app.device,app.bufferMemory[2]);
    for(size_t i = 0; i < c_size; ++i) {
        ASSERT_NEAR(expected[i],out[i],EPSILON);
    }
}
// Random value test
TEST(DGEMM, random) {
    srand((unsigned int)time(NULL));

    size_t const numPushConstants = 5;

    constexpr size_t const m = LOWER_MIN_SIZE + (linearCongruentialGenerator(12) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));
    constexpr size_t const k = LOWER_MIN_SIZE + (linearCongruentialGenerator(13) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));
    constexpr size_t const n = LOWER_MIN_SIZE + (linearCongruentialGenerator(14) % (LOWER_MAX_SIZE - LOWER_MIN_SIZE + 1));

    size_t const a_size = m * k;
    size_t const b_size = k * n;
    size_t const c_size = m * n;

    std::array<double,a_size> A;
    std::array<double,b_size> B;
    std::array<double,c_size> C;
    std::array<double,c_size> C_cpu; // cpu side verison of C which retains the original values
    for(size_t i = 0; i < a_size; ++i) {
        A[i] = double(rand())/double(RAND_MAX);
    }
    for(size_t i = 0; i < b_size; ++i) {
        B[i] = double(rand())/double(RAND_MAX);
    }
    for(size_t i = 0; i < c_size; ++i) {
        C[i] = double(rand())/double(RAND_MAX);
        C_cpu[i] = C[i];
    }

    auto data = std::make_tuple(
        std::move(A),
        std::move(B),
        std::move(C)
    );

    constexpr double const alpha = randToFloat(linearCongruentialGenerator(15));
    constexpr double const beta = randToFloat(linearCongruentialGenerator(16));
    static std::array<std::variant<uint32_t,float,double>,numPushConstants> const pushConstants = {
        alpha,
        beta,
        static_cast<uint32_t>(m),
        static_cast<uint32_t>(k),
        static_cast<uint32_t>(n)
    };

    char const shader[] = "../../../glsl/dgemm.spv";

    ComputeApp app = ComputeApp<numPushConstants,pushConstants,double,a_size,b_size,c_size>(
        shader,
        data, // Buffer data
        std::array<size_t,3> { 1,1,1 }, // Invocations
        std::array<size_t,3> { WORKGROUP_SIZE,1,1 } // Workgroup sizes
    );

    std::array<double,c_size> expected;
    for(size_t m_index = 0; m_index < m; ++m_index) {
        const size_t A_row = k * m_index;
        for(size_t n_index = 0; n_index < n; ++n_index) {
            double temp = 0;
            for(size_t k_index = 0; k_index < k; ++k_index) {
                temp += std::get<0>(data)[A_row + k_index] * std::get<1>(data)[n * k_index + n_index];
                // std::cout << std::get<0>(data)[A_row + k_index] << '*' << std::get<1>(data)[n * k_index + n_index] << ' ';
            }
            // std::cout << temp << ' ';
            const size_t C_index = n * m_index + n_index;
            expected[C_index] = alpha * temp + beta * C_cpu[C_index];
            // std::cout << alpha << '*' << temp << '+' << beta << '*' << C_cpu[C_index] << '=' << expected[C_index] << std::endl;
        }
    }
    //assert(false);

    double* out = Utility::map<double*>(app.device,app.bufferMemory[2]);
    for(size_t i = 0; i < c_size; ++i) {
        ASSERT_NEAR(expected[i],out[i],2*EPSILON);
    }
}