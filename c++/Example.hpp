#pragma once

#include <vulkan/vulkan.h> // Vulkan
#include <vector> // vector
#include <iostream> // cout
#include <string.h> // strcmp
#include <algorithm> // find_if
#include <optional> // optional
#include <assert.h> // assert
#include <cmath> // ceil

// If debug, enable validation layers
#ifdef NDEBUG
const auto enableValidationLayers = std::nullopt;
#else
const auto enableValidationLayers = std::optional<char const*>{"VK_LAYER_KHRONOS_validation"};
#endif

// Macro to check Vulkan result
#define VK_CHECK_RESULT(f) 																\
{																						\
    VkResult res = (f);																	\
    if (res != VK_SUCCESS)																\
    {																					\
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__);  \
        assert(res == VK_SUCCESS);														\
    }																					\
}

enum Reduction { Max, Sum };

// All infomation which varies when using different compute shaders
// Sharing buffers
class ShaderRunInfo {
    // --------------------------------------------------
    // Members
    // --------------------------------------------------
    public:
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;

        VkDescriptorSet descriptorSet;

        VkShaderModule computeShaderModule;

        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
    // --------------------------------------------------
    // Methods
    // --------------------------------------------------
    public:
        ShaderRunInfo() = default;
        ShaderRunInfo(
            VkDevice& device,
            VkBuffer*& buffers,
            uint32_t queueFamilyIndex,
            VkQueue& queue,
            char const* shaderFile,
            uint32_t numBuffers,
            float* pushConstants,
            uint32_t numPushConstants,
            uint32_t const* dims, // [x,y,z],
            uint32_t const* dimLengths, // [local_size_x, local_size_y, local_size_z]
            std::optional<uint32_t const*> bufferSizes = std::nullopt
        );

        // Creates descriptor set layout
        void createDescriptorSetLayout(
            VkDevice& device,
            uint32_t numBuffers,
            VkDescriptorSetLayout* descriptorSetLayout
        );

        // Creates descriptor set
        void createDescriptorSet(
            VkDevice& device,
            uint32_t storageBuffers,
            VkDescriptorPool* descriptorPool,
            VkDescriptorSetLayout* descriptorSetLayout,
            VkBuffer* buffers,
            std::optional<uint32_t const*> bufferSizes = std::nullopt
        );

        // Reads shader file
        uint32_t* readShader(uint32_t& length, const char* filename);

        // Creates compute pipeline
        void createComputePipeline(
            VkDevice& device,
            char const* shaderFile,
            VkShaderModule* computeShaderModule,
            VkDescriptorSetLayout* descriptorSetLayout,
            VkPipelineLayout* pipelineLayout,
            VkPipeline* pipeline,
            float const* pushConstants,
            uint32_t numPushConstants
        );

        // Creates command buffer
        void createCommandBuffer(
            uint32_t queueFamilyIndex,
            VkDevice& device,
            VkCommandPool* commandPool,
            VkCommandBuffer* commandBuffer,
            VkPipeline& pipeline,
            VkPipelineLayout& pipelineLayout,
            float const* pushConstants,
            uint32_t numPushConstants,
            uint32_t const* dims, // [x,y,z],
            uint32_t const* dimLengths // [local_size_x, local_size_y, local_size_z]
        );

        // Runs command buffer
        void runCommandBuffer(
            VkCommandBuffer* commandBuffer,
            VkDevice& device,
            VkQueue& queue
        );
};

class ComputeApp {
    // --------------------------------------------------
    // Public members
    // --------------------------------------------------
    public:
        VkDevice device;
        VkDeviceMemory* bufferMemories;
    // --------------------------------------------------
    // Private members
    // --------------------------------------------------
    private:
        // In order of usage
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        uint32_t queueFamilyIndex;
        VkQueue queue;

        VkBuffer* buffers;

        ShaderRunInfo baseShaderRunInfo;
        ShaderRunInfo* reductionShaderInfo;

        uint32_t numBuffers; // neccessary for destruction
        uint32_t numReductions; //neccessary for destruction

    // --------------------------------------------------
    // Public methods
    // --------------------------------------------------
    public:
        static void shrink(
            VkDevice& device,
            VkDeviceMemory& bufferMemory,
            uint32_t const size,
            uint32_t const stride,
            uint32_t const segments
        );
        static void clear(
            VkDevice& device,
            VkDeviceMemory& bufferMemory,
            uint32_t const start,
            uint32_t const stop
        );
        static float* map(VkDevice& device, VkDeviceMemory& bufferMemory);
        static void print(VkDevice& device,VkDeviceMemory& bufferMemory, uint32_t size);

        ComputeApp(
            char const* shaderFile,
            uint32_t* bufferSizes,
            uint32_t const numBuffers,
            float**& bufferData,
            float* pushConstants,
            uint32_t numPushConstants,
            uint32_t* dims, // [x,y,z],
            uint32_t const* dimLengths, // [local_size_x, local_size_y, local_size_z]
            bool requiresAtomic,
            bool reduction = false
        );
        ~ComputeApp();
    // --------------------------------------------------
    // Private methods
    // --------------------------------------------------
    private:
        // Gets Vulkan instance
        void createInstance(VkInstance& instance, bool const requiresAtomic);
        
        // Gets physical device
        void getPhysicalDevice(VkInstance const& instance, VkPhysicalDevice& physicalDevice);

        // Gets index of 1st queue family which supports compute
        uint32_t getComputeQueueFamilyIndex();
        // Creates logical device
        void createDevice(
            VkPhysicalDevice const& physicalDevice,
            uint32_t& queueFamilyIndex,
            VkDevice& device,
            VkQueue& queue
        );

        // Finds memory type with given properties
        uint32_t findMemoryType(uint32_t const memoryTypeBits, VkMemoryPropertyFlags const properties);

        // Creates buffer
        void createBuffer(
            VkDevice const& device,
            uint32_t const size,
            VkBuffer* buffer,
            VkDeviceMemory* bufferMemory
        );

        // Creates buffers
        void createBuffers(
            VkDevice const& device,
            uint32_t const* bufferSizes,
            uint32_t const numBuffers,
            VkBuffer*& buffers,
            VkDeviceMemory*& bufferMemories
        );

        // Fills buffers with data
        // Must be after `createBuffers` but before `createComputePipeline`
        void fillBuffers(
            VkDevice const & device,
            float**& bufferData,
            VkDeviceMemory*& bufferMemories,
            uint32_t const numBuffers,
            uint32_t const* bufferSizes
        );

        
};

int TwentyOne();