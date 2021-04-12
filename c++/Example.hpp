#pragma once

#include <vulkan/vulkan.h> // Vulkan
#include <optional> // std::optional
#include <string.h> // strcmp
#include <assert.h> // assert
#include <cmath> // ceil
#include <variant> // std::variant
#include <array> // std::array
#include <numeric> // std::accumulate
#include <algorithm> // std::for_each
#include <cstring> // std::memcpy

#include <iostream>
#include <tuple> // std::tuple
#include <limits>
#include <chrono>

#ifdef NDEBUG
const std::optional<char const*> enableValidationLayers = std::nullopt;
#else
const std::optional<char const*> enableValidationLayers = std::optional<char const*>{"VK_LAYER_KHRONOS_validation"};
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

namespace Utility {
    // Creates Vulkan instance
    void createInstance(VkInstance& instance);
    // Gets physical device
    void getPhysicalDevice(VkInstance const& instance, VkPhysicalDevice& physicalDevice);
    // Gets an index to a queue family
     size_t getComputeQueueFamilyIndex(VkPhysicalDevice const& physicalDevice);
    // Creates logical device
    void createDevice(
        VkPhysicalDevice const& physicalDevice,
        size_t& queueFamilyIndex,
        VkDevice& device,
        VkQueue& queue
    );
    // Finds the memory type by which we can access memory allocated from the heap
     size_t findMemoryType(
        VkPhysicalDevice const& physicalDevice,
        size_t const memoryTypeBits,
        VkMemoryPropertyFlags const properties
    );
    // Creates buffer
    template<typename T, size_t Size>
    void createBuffer(
        VkPhysicalDevice const& physicalDevice,
        VkDevice const& device,
        std::array<T,Size>& bufferValues,
        VkBuffer * const buffer,
        VkDeviceMemory * const bufferMemory
    ) {
        // Buffer info
        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            // buffer size in bytes.
            .size = sizeof(T)*Size,
            // buffer is used as a storage buffer (and is thus accessible in a shader).
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            // buffer is exclusive to a single queue family at a time. 
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        // Constructs buffer
        VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer));

        // Buffers do not allocate memory upon instantiaton, we must do it manually
        
        // Gets buffer memory size and offset
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);
        
        // Memory info
        VkMemoryAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size  // Size in bytes
        };

        allocateInfo.memoryTypeIndex = findMemoryType(
            physicalDevice,
            // Specifies memory types supported for the buffer
            memoryRequirements.memoryTypeBits,
            // Sets memory must have the properties:
            //  `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` Easily view
            //  `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT` Read from GPU to CPU
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );

        // Allocates memory
        VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, nullptr, bufferMemory));

        // Binds buffer to allocated memory
        VK_CHECK_RESULT(vkBindBufferMemory(device, *buffer, *bufferMemory, 0));
    }
    // Creates buffers
    template<size_t I = 0, typename T, size_t... Sizes>
    void createBuffers(
        VkPhysicalDevice const & physicalDevice,
        VkDevice const & device,
        std::tuple<std::array<T, Sizes>...> & bufferValues,
        std::array<VkBuffer, sizeof...(Sizes)> & buffer,
        std::array<VkDeviceMemory, sizeof...(Sizes)> & bufferMemory
    ) {
        if constexpr(I == sizeof...(Sizes)) { return; }
        else {
            // Creates buffer
            Utility::createBuffer(physicalDevice, device, std::get<I>(bufferValues), &buffer[I], &bufferMemory[I]);
            // Iterate
            Utility::createBuffers<I+1>(physicalDevice,device,bufferValues,buffer,bufferMemory);
        }
    }
    // Fills a buffer with given data
    template <typename T, size_t Size>
    void fillBuffer(
        VkDevice const & device,
        VkDeviceMemory& bufferMemory,
        std::array<T,Size> & bufferData
    )  {
        void* data = nullptr;
        // Maps buffer memory into RAM
        vkMapMemory(device, bufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
        // Fills buffer memory
        memcpy(data, bufferData.data(), Size * sizeof(T));
        // Un-maps buffer memory from RAM to device memory
        vkUnmapMemory(device, bufferMemory);
    }
    template <size_t I = 0, typename T, size_t... Sizes>
    void fillBuffers(
        VkDevice const & device,
        std::array<VkDeviceMemory,sizeof...(Sizes)> bufferMemory,
        std::tuple<std::array<T,Sizes>...> & data
    )  {
        if constexpr(I == sizeof...(Sizes)) { return; }
        else {
            Utility::fillBuffer(device,bufferMemory[I],std::get<I>(data));
            Utility::fillBuffers<I+1>(device,bufferMemory,data);
        }
    }
    // Creates descriptor set layout
    template<size_t NumBuffers>
    void createDescriptorSetLayout(
        VkDevice const& device, 
        VkDescriptorSetLayout* descriptorSetLayout
    )  {
        std::array<VkDescriptorSetLayoutBinding,NumBuffers> binding;
        for(size_t i = 0; i < NumBuffers; ++i){
            binding[i].binding = i; // `layout(binding = 0)`
            binding[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            // Specifies the number buffers of a binding
            //  `layout(binding=0) buffer Buffer { uint x[]; }` or
            //   `layout(binding=0) buffer Buffer { uint x[]; } buffers[1]` would equal 1
            //
            //  `layout(binding=0) buffer Buffer { uint x[]; } buffers[3]` would equal 3,
            //   in affect saying we have 3 buffers of the same format (`buffers[0].x` etc.).
            binding[i].descriptorCount = 1;
            binding[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        }   
        
        // Descriptor set layout options
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            // `bindingCount` specifies length of `pBindings` array, in this case 1.
            .bindingCount = static_cast<uint32_t>(NumBuffers),
            // array of `VkDescriptorSetLayoutBinding`s
            .pBindings = binding.data()
        };
        
        // Create the descriptor set layout. 
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(
            device, &descriptorSetLayoutCreateInfo, nullptr, descriptorSetLayout
        ));
    }
    // Creates descriptor set
    template<size_t NumBuffers>
    void createDescriptorSet(
        VkDevice const& device,
        VkDescriptorPool* descriptorPool,
        VkDescriptorSetLayout* descriptorSetLayout,
        std::array<VkBuffer,NumBuffers>& buffer,
        VkDescriptorSet& descriptorSet
    ) {
        // Descriptor type and number
        VkDescriptorPoolSize descriptorPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = static_cast<uint32_t>(NumBuffers) // Number of descriptors
        };
        // Creates descriptor pool
        // A pool allocates a number of descriptors of each type
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1, // max number of sets that can be allocated from this pool
            .poolSizeCount = 1, // length of `pPoolSizes`
            .pPoolSizes = &descriptorPoolSize // pointer to array of `VkDescriptorPoolSize`
        };
        // create descriptor pool.
        VK_CHECK_RESULT(vkCreateDescriptorPool(
            device, &descriptorPoolCreateInfo, nullptr, descriptorPool
        ));

        // Specifies options for creation of multiple of descriptor sets
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            // pool from which sets will be allocated
            .descriptorPool = *descriptorPool, 
            // number of descriptor sets to implement (length of `pSetLayouts`)
            .descriptorSetCount = 1, 
            // pointer to array of descriptor set layouts
            .pSetLayouts = descriptorSetLayout 
        };
        // allocate descriptor set.
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

        // Binds descriptors to buffers
        std::array<VkDescriptorBufferInfo,NumBuffers> binding;
        for(size_t i = 0; i < NumBuffers; ++i){
            binding[i].buffer = buffer[i];
            binding[i].offset = 0;
            binding[i].range = VK_WHOLE_SIZE; // set to whole size of buffer
        }

        // Binds descriptors from descriptor sets to buffers
        VkWriteDescriptorSet writeDescriptorSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            // write to this descriptor set.
            .dstSet = descriptorSet,
            // update 1 descriptor respective set (we only have 1).
            .descriptorCount = static_cast<uint32_t>(NumBuffers),
            // buffer type.
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            // respective buffer.
            .pBufferInfo = binding.data()
        };
        
        // perform the update of the descriptor set.
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    }
    // Reads shader file
    std::pair<size_t, uint32_t*> readShader(char const* filename);

    template <size_t NumPushConstants>
    constexpr size_t pushConstantsSize(std::array<std::variant<uint32_t, float, double>, NumPushConstants> const& pushConstants) {
        auto size_fn = [](auto const& var) -> size_t {
            using T = std::decay_t<decltype(var)>;
            return sizeof(T);
        };
        return static_cast<size_t>(std::accumulate(pushConstants.cbegin(), pushConstants.cend(),
            std::size_t{ 0 },
            [size_fn](std::size_t acc, auto const var) { return acc + std::visit(size_fn,var); }
        ));
    }

    // Creates compute pipeline
    template <size_t PushConstantSize>
    void createComputePipeline(
        VkDevice const& device,
        char const* shaderFile,
        VkShaderModule* computeShaderModule,
        VkDescriptorSetLayout* descriptorSetLayout,
        VkPipelineLayout* pipelineLayout,
        VkPipeline* pipeline
    ) {
        // Creates shader module (just a wrapper around our shader)
        // std::pair<size_t,size_t*> file = readShader(shaderFile); // (length,bytes)
        // VkShaderModuleCreateInfo createInfo = {
        //     .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        //     .codeSize = file.first,
        //     .pCode = file.second
        // };
        auto [fileLength, fileBytes] = readShader(shaderFile); // (length,bytes)
        VkShaderModuleCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = fileLength,
            .pCode = fileBytes
        };

        VK_CHECK_RESULT(vkCreateShaderModule(
            device, &createInfo, nullptr, computeShaderModule
        ));

        // A compute pipeline is very simple compared to a graphics pipeline.
        // It only consists of a single stage with a compute shader.

        // The pipeline layout allows the pipeline to access descriptor sets. 
        // So we just specify the descriptor set layout we created earlier.
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1, // 1 descriptor set
            .pSetLayouts = descriptorSetLayout // the 1 descriptor set 
        };
        if constexpr (PushConstantSize > 0) {
            pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pipelineLayoutCreateInfo.pPushConstantRanges = new VkPushConstantRange {
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .offset = 0,
                .size = PushConstantSize
            };
        }
        
        VK_CHECK_RESULT(vkCreatePipelineLayout(
            device, &pipelineLayoutCreateInfo, nullptr, pipelineLayout
        ));

        // We specify the compute shader stage, and it's entry point(main).
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT, // Shader type
            .module = *computeShaderModule, // Shader module
            .pName = "main" // Shader entry point
        };

        // Set our pipeline options
        VkComputePipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = shaderStageCreateInfo,
            .layout = *pipelineLayout
        };

        // Create compute pipeline
        VK_CHECK_RESULT(vkCreateComputePipelines(
            device, VK_NULL_HANDLE,
            1, &pipelineCreateInfo,
            nullptr, pipeline
        ));
    }

    // Creates command buffer
    template <size_t PushConstantSize, size_t NumPushConstants>
    void createCommandBuffer(
        size_t queueFamilyIndex,
        VkDevice& device,
        VkCommandPool* commandPool,
        VkCommandBuffer* commandBuffer,
        VkPipeline& pipeline,
        VkPipelineLayout& pipelineLayout,
        VkDescriptorSet& descriptorSet,
        std::array<size_t, 3> dims, // [x,y,z],
        std::array<size_t, 3> dimLengths, // [local_size_x, local_size_y, local_size_z]
        std::array<std::variant<uint32_t, float, double>, NumPushConstants> const & pushConstants
    ) {
        // Creates command pool
        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = static_cast<uint32_t>(queueFamilyIndex) // Sets queue family
        };
        VK_CHECK_RESULT(vkCreateCommandPool(
            device, &commandPoolCreateInfo, nullptr, commandPool
        ));

        // Allocates command buffer
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = *commandPool,  // Pool to allocate from
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1  // Allocates 1 command buffer. 
        };
        VK_CHECK_RESULT(vkAllocateCommandBuffers(
            device, &commandBufferAllocateInfo, commandBuffer
        ));

        // Allocated command buffer options
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            // Buffer only submitted once
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        // Start recording commands
        VK_CHECK_RESULT(vkBeginCommandBuffer(*commandBuffer, &beginInfo));

        // Binds pipeline (our functions)
        vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        // Binds descriptor set (our data)
        vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        // Sets push constants
        if constexpr (PushConstantSize > 0) {
            std::array<std::byte, PushConstantSize> bytes;
            size_t byteCounter = 0;
            std::for_each(pushConstants.cbegin(), pushConstants.cend(), [&](auto const& var) {
                std::visit([&] (auto const& var) {
                    using T = std::decay_t<decltype(var)>;
                    std::memcpy(bytes.data() + byteCounter, static_cast<void const*>(&var), sizeof(T));
                    byteCounter += sizeof(T);
                }, var);
            });
            
            vkCmdPushConstants(
                *commandBuffer, 
                pipelineLayout, 
                VK_SHADER_STAGE_COMPUTE_BIT, 
                0, 
                PushConstantSize, 
                static_cast<void*>(bytes.data())
            );
        }

        auto const [x,y,z] = std::make_tuple(
            ceil(dims[0] / static_cast<float>(dimLengths[0])),
            ceil(dims[1] / static_cast<float>(dimLengths[1])),
            ceil(dims[2] / static_cast<float>(dimLengths[2]))
        );

        // Sets invocations
        vkCmdDispatch(
            *commandBuffer,
            x,y,z
        );

        // End recording commands
        VK_CHECK_RESULT(vkEndCommandBuffer(*commandBuffer));
    }

    // Runs command buffer
    void runCommandBuffer(
        VkCommandBuffer* commandBuffer,
        VkDevice const& device,
        VkQueue const& queue
    );

    // Maps buffer to CPU memory
    template<typename T>
    T map(
        VkDevice& device,
        VkDeviceMemory& bufferMemory
    ) {
        void* data = nullptr;
        vkMapMemory(device, bufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
        return static_cast<T>(data);
    }
}

template <
    size_t NumPushConstants,
    std::array<std::variant<uint32_t,float,double>, NumPushConstants> const& pushConstant,
    typename T,
    size_t... BufferSizes
>
class ComputeApp {
    // -------------------------------------------------
    // Private members
    // -------------------------------------------------
    public:
        VkInstance instance;                                            // Vulkan instance.
        VkPhysicalDevice physicalDevice;                                // Physical device (e.g. GPU).
        VkDevice device;                                                // Logical device by which we connect to our physical device.
        size_t queueFamilyIndex;                                        // Index to a queue family.
        VkQueue queue;                                                  // Queue.
        size_t numHeldBuffers;                                          // Number of buffers (necessary for destruction).
        std::array<VkBuffer, sizeof...(BufferSizes)> buffer;             // Buffers.
        std::array<VkDeviceMemory, sizeof...(BufferSizes)> bufferMemory; // Buffer memories.
        VkDescriptorSetLayout descriptorSetLayout;                      // Layout of a descriptor set.
        VkDescriptorPool descriptorPool;                                // Pool from which to pull descriptor sets.
        VkDescriptorSet descriptorSet;                                  // Descriptor set.
        VkShaderModule computeShaderModule;                             // Shader.
        VkPipelineLayout pipelineLayout;                                // Layout for a pipeline.
        VkPipeline pipeline;                                            // Pipeline.
        VkCommandPool commandPool;                                      // Pool from which to pull command buffer.
        VkCommandBuffer commandBuffer;                                  // Command buffer.
    // -------------------------------------------------
    // Public methods
    // -------------------------------------------------
    public:
        ComputeApp(
            char const* shaderFile,
            std::tuple<std::array<T, BufferSizes>...> & buffers,
            std::array<size_t, 3> dims, // [x,y,z],
            std::array<size_t, 3> dimLengths // [local_size_x, local_size_y, local_size_z]
        )  {
            constexpr size_t const numBuffers = sizeof...(BufferSizes);
            
            this->numHeldBuffers = numBuffers;

            // Initialize vulkan:
            Utility::createInstance(this->instance);

            // Gets physical device
            Utility::getPhysicalDevice(this->instance, this->physicalDevice);

            // Gets logical device
            Utility::createDevice(this->physicalDevice, this->queueFamilyIndex, this->device, this->queue);

            // Creates buffers
            Utility::createBuffers(
                this->physicalDevice,
                this->device,
                buffers,
                this->buffer,
                this->bufferMemory
            );

            Utility::fillBuffers(this->device,this->bufferMemory,buffers);

            // Creates descriptor set layout
            Utility::createDescriptorSetLayout<numBuffers>(this->device,&this->descriptorSetLayout);

            // Creates descriptor set
            Utility::createDescriptorSet(this->device,&this->descriptorPool,&this->descriptorSetLayout,this->buffer,this->descriptorSet);

            constexpr size_t const pcSize = Utility::pushConstantsSize(pushConstant);

            // Creates compute pipeline
            Utility::createComputePipeline<pcSize>(
                this->device,
                shaderFile,
                &this->computeShaderModule,
                &this->descriptorSetLayout,
                &this->pipelineLayout,
                &this->pipeline
            );

            
            // Creates command buffer
            Utility::createCommandBuffer<pcSize>(
                this->queueFamilyIndex,
                this->device,
                &this->commandPool,
                &this->commandBuffer,
                this->pipeline,
                this->pipelineLayout,
                this->descriptorSet,
                dims,
                dimLengths,
                pushConstant
            );

            Utility::runCommandBuffer(
                &this->commandBuffer,
                this->device,
                this->queue
            );
        }
        ~ComputeApp()  {
            for(size_t i=0;i<numHeldBuffers;++i) {
                vkFreeMemory(device, bufferMemory[i], nullptr);
                vkDestroyBuffer(device, buffer[i], nullptr);
            }

            vkDestroyShaderModule(device, computeShaderModule, nullptr);
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            vkDestroyPipeline(device, pipeline, nullptr);
            vkDestroyCommandPool(device, commandPool, nullptr);

            vkDestroyDevice(device, nullptr);
            vkDestroyInstance(instance, nullptr);		
        }
};

constexpr float randToFloat(uint64_t const x) {
    return static_cast<float>(x) / static_cast<float>(std::numeric_limits<uint64_t>::max());
}

template<
    uint64_t m = 4294967296, // 2^32
    uint64_t a = 1664525,
    uint64_t c = 1013904223
>
constexpr uint64_t linearCongruentialGenerator(
    size_t const n,
    uint64_t const s =
        100000*(*__TIME__-'0')+
        10000*(*(__TIME__+1)-'0')+
        1000*(*(__TIME__+3)-'0')+
        100*(*(__TIME__+4)-'0')+
        10*(*(__TIME__+6)-'0')+
        (*(__TIME__+7)-'0')
) {
    uint64_t x = s;
    for(size_t i = 0; i < n; ++i) {
        x = (a * x + c) % m;
    }
    return x;
}
