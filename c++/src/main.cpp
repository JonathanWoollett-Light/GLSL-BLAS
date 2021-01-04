
#include <vulkan/vulkan.h>

#include <iostream>

#include <vector>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <cmath>

const int WIDTH = 3200; // Size of rendered mandelbrot set.
const int HEIGHT = 2400; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

const int MATRIX_SIZE = 300;
const int VECTOR_SIZE = MATRIX_SIZE * MATRIX_SIZE;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f) 																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);																		\
    }																									\
}

class ComputeApplication {
private:
    
    // Order of usage
    std::vector<const char*> enabledLayers;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    uint32_t queueFamilyIndex;
    VkDevice device;
    VkQueue queue;

    VkBuffer* buffers;
    VkDeviceMemory* bufferMemories;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkShaderModule computeShaderModule;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // Newly unused
    // ------------------
    VkDescriptorSet descriptorSet;

public:
    void run(
        uint32_t const * bufferSizes,
        uint32_t numBuffers,
        float**& bufferData,
        float* pushConstants,
        uint32_t numPushConstants,
        int const* dims, // [x,y,z],
        int const* dimLengths // [local_size_x, local_size_y, local_size_z]
    ) {
        std::cout << "in:" << std::endl;
        for (int i = 0; i < numBuffers; ++i) {
            std::cout << '\t';
            for (int j = 0; j < bufferSizes[i]; ++j) {
                std::cout << bufferData[i][j] << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << '\t' << '[' << ' ';
        for (int i = 0; i < numPushConstants; ++i) {
            std::cout << pushConstants[i] << ' ';
        }
        std::cout << ']';
        std::cout << std::endl << std::endl;

        // Initialize vulkan:
        createInstance(enabledLayers,instance);

        // Gets physical device
        findPhysicalDevice(instance, physicalDevice);

        // Gets logical device
        createDevice(physicalDevice, queueFamilyIndex, device, queue);

        // Creates buffers
        createBuffers(device, bufferSizes, numBuffers, buffers, bufferMemories);

        // Creates descriptor set layout
        createDescriptorSetLayout(device, numBuffers, &descriptorSetLayout);

        // Create descriptor set
        createDescriptorSet(device,numBuffers, &descriptorPool,&descriptorSetLayout,buffers, bufferSizes);

        // Fills buffers
        fillBuffers(device, bufferData, bufferMemories, numBuffers, bufferSizes);

        // Creates compute pipeline
        createComputePipeline(
            device,
            "../glsl/sscal.spv",
            &computeShaderModule,
            &descriptorSetLayout,
            &pipelineLayout,
            &pipeline,
            pushConstants,
            numPushConstants
        );

        // Create command buffer
        createCommandBuffer(
            queueFamilyIndex,
            device,
            &commandPool,
            &commandBuffer,
            pipeline,
            pipelineLayout,
            pushConstants,
            numPushConstants,
            dims,
            dimLengths
        );

        // Finally, run the recorded command buffer.
        runCommandBuffer(&commandBuffer,device,queue);

        printOutput(device, bufferMemories[0], bufferSizes[0]);

        // Clean up all vulkan resources.
        cleanup();
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData) {

        printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);

        return VK_FALSE;
     }

    // Initiates Vulkan instance
    void createInstance(std::vector<const char*> &enabledLayers, VkInstance& instance) {
        std::vector<const char*> enabledExtensions;

        if (enableValidationLayers) {
            /*
            We get all supported layers with vkEnumerateInstanceLayerProperties.
            */
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, NULL);

            std::vector<VkLayerProperties> layerProperties(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

            /*
            And then we simply check if VK_LAYER_LUNARG_standard_validation is among the supported layers.
            */
            bool foundLayer = false;
            for (VkLayerProperties prop : layerProperties) {

                if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0) {
                    foundLayer = true;
                    break;
                }
            }

            if (!foundLayer) {
                throw std::runtime_error("Layer VK_LAYER_KHRONOS_validation not supported\n");
            }
            enabledLayers.push_back("VK_LAYER_KHRONOS_validation"); // Alright, we can use this layer.

            /*
            We need to enable an extension named VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
            in order to be able to print the warnings emitted by the validation layer.

            So again, we just check if the extension is among the supported extensions.
            */

            uint32_t extensionCount;

            vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
            std::vector<VkExtensionProperties> extensionProperties(extensionCount);
            vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

            bool foundExtension = false;
            for (VkExtensionProperties prop : extensionProperties) {
                if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) {
                    foundExtension = true;
                    break;
                }

            }

            if (!foundExtension) {
                throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
            }
            enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        VkInstanceCreateInfo createInfo = {};
        {
            VkApplicationInfo applicationInfo = {};
            {
                applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                applicationInfo.apiVersion = VK_API_VERSION_1_0;
            }
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &applicationInfo;
            createInfo.enabledLayerCount = enabledLayers.size();
            createInfo.ppEnabledLayerNames = enabledLayers.data();
            createInfo.enabledExtensionCount = enabledExtensions.size();
            createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        }
    
        // Creates instance
        VK_CHECK_RESULT(vkCreateInstance(
            &createInfo,
            NULL,
            &instance)
        );
    }

    void findPhysicalDevice(VkInstance& instance, VkPhysicalDevice& physicalDevice) {
        // Gets number of physical devices
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
        // presumes `deviceCount > 0`

        // Gets physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Picks 1st
        physicalDevice = devices[0];
    }

    // Returns an index of a queue family that supports compute
    uint32_t getComputeQueueFamilyIndex() {
        // Gets number of queue families
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

        // Gets queue families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        // Picks 1st queue family which supports compute
        for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
            VkQueueFamilyProperties props = queueFamilies[i];
            // If queue family supports compute
            if (props.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                return i;
            }
        }
        throw std::runtime_error("No compute queue family");
    }
    // Gets logical device
    void createDevice(
        VkPhysicalDevice& physicalDevice,
        uint32_t& queueFamilyIndex,
        VkDevice& device,
        VkQueue& queue
    ) {
        // Device info
        VkDeviceCreateInfo deviceCreateInfo = {};
        {   
            // Device queue info
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            {
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
                queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
                queueCreateInfo.pQueuePriorities = new float(1.0);
            }
            // Device features
            VkPhysicalDeviceFeatures deviceFeatures = {};

            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
            deviceCreateInfo.queueCreateInfoCount = 1;
            deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        }

        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)); // create logical device.

        // Get handle to queue 0 in `queueFamilyIndex` queue family
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    }

    // find memory type with desired properties.
    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memoryProperties;

        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        /*
        How does this search work?
        See the documentation of VkPhysicalDeviceMemoryProperties for a detailed description. 
        */
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            if ((memoryTypeBits & (1 << i)) &&
                ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
                return i;
        }
        return -1;
    }

    // Creates buffers
    void createBuffers(
        VkDevice& device,
        uint32_t const* bufferSizes,
        uint32_t numBuffers,
        VkBuffer*& buffers,
        VkDeviceMemory*& bufferMemories
    ) {
        buffers = new VkBuffer[numBuffers];
        bufferMemories = new VkDeviceMemory[numBuffers];
        for (uint32_t i = 0; i < numBuffers; ++i) {
            createBuffer(device, bufferSizes[i], &buffers[i], &bufferMemories[i]);
        }
    }
    // Creates buffer
    void createBuffer(
        VkDevice& device,
        uint32_t size,
        VkBuffer* buffer,
        VkDeviceMemory* bufferMemory
    ) {
        // Buffer info
        VkBufferCreateInfo bufferCreateInfo = {};
        {
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.size = size; // buffer size in bytes. 
            bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time. 
        }

        VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, buffer)); // Constructs buffer

        // Buffers do not allocate memory upon construction, we must do it manually
        
        // Gets buffer memory size and offset
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);
        
        // Sets buffer options
        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = memoryRequirements.size; // Bytes

        allocateInfo.memoryTypeIndex = findMemoryType(
            // Sets memory must supports all the operations our buffer memory supports
            memoryRequirements.memoryTypeBits,
            // Sets memory must have the properties:
            //  `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` Can more easily view
            //  `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT` Can read from GPU to CPU
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );

        // Allocates memory
        VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, bufferMemory));

        // Binds buffer to allocated memory
        VK_CHECK_RESULT(vkBindBufferMemory(device, *buffer, *bufferMemory, 0));
    }
    
    // Must be after `createBuffers` but before `createComputePipeline`
    void fillBuffers(
        VkDevice& device,
        float**& bufferData,
        VkDeviceMemory*& bufferMemories,
        uint32_t numBuffers,
        uint32_t const* bufferSizes
    ) {
        for (uint32_t i = 0; i < numBuffers; ++i) {
            void* data = nullptr;
            vkMapMemory(device, bufferMemories[i], 0, VK_WHOLE_SIZE, 0, &data);
            memcpy(data, bufferData[i], (size_t)(bufferSizes[i] * sizeof(float)));
            vkUnmapMemory(device, bufferMemories[i]);
        }
    }

    // Create descriptor set layout
    void createDescriptorSetLayout(
        VkDevice& device,
        uint32_t numBuffers,
        VkDescriptorSetLayout* descriptorSetLayout
    ) {
        
        // Descriptor set layout options
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        {
            VkDescriptorSetLayoutBinding* bindings = new VkDescriptorSetLayoutBinding[numBuffers];
            for (uint32_t i = 0; i < numBuffers; ++i) {
                bindings[i].binding = i; // `layout(binding = i)`
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[i].descriptorCount = 1; // TODO Wtf does this do? Number of items in buffer maybe?
                bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCreateInfo.bindingCount = numBuffers; // 1 descriptor/bindings in this descriptor set
            descriptorSetLayoutCreateInfo.pBindings = bindings;
        }
        
        // Create the descriptor set layout. 
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, descriptorSetLayout));
    }
    
    // Creates descriptor set
    void createDescriptorSet(
        VkDevice& device,
        uint32_t storageBuffers,
        VkDescriptorPool* descriptorPool,
        VkDescriptorSetLayout* descriptorSetLayout,
        VkBuffer* buffers,
        uint32_t const* bufferSizes
    ) {
        // Creates descriptor pool
        // A pool implements a number of descriptors of each type 
        //  `VkDescriptorPoolSize` specifies for each descriptor type the number to hold
        // A descriptor set is initialised to contain all descriptors defined in a descriptor pool
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
        {
            // Descriptor type and number
            VkDescriptorPoolSize descriptorPoolSize = {};
            {
                descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // Descriptor type
                descriptorPoolSize.descriptorCount = storageBuffers; // Number of descriptors
            }
            descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolCreateInfo.maxSets = 1; // max number of sets that can be allocated from this pool
            descriptorPoolCreateInfo.poolSizeCount = 1; // length of `pPoolSizes`
            descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize; // pointer to array of `VkDescriptorPoolSize`
        }

        // create descriptor pool.
        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, descriptorPool));

        // Specifies options for creation of multiple of descriptor sets
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        {
            descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetAllocateInfo.descriptorPool = *descriptorPool; // pool from which sets will be allocated
            descriptorSetAllocateInfo.descriptorSetCount = 1; // number of descriptor sets to implement (also length of `pSetLayouts`)
            descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayout; // pointer to array of descriptor set layouts
        }
        

        // allocate descriptor set.
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

        // Gets buffer memory size and offset
        // VkMemoryRequirements memoryRequirements;
        // vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

        // Binds descriptors from our descriptor sets to our buffers
        VkWriteDescriptorSet writeDescriptorSet = {};
        {
            // Binds descriptors to buffers
            VkDescriptorBufferInfo* bindings = new VkDescriptorBufferInfo[storageBuffers];
            
            for (uint32_t i = 0; i < storageBuffers; ++i) {
                bindings[i].buffer = buffers[i];
                bindings[i].offset = 0;
                bindings[i].range = bufferSizes[i];
            }

            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = descriptorSet; // write to this descriptor set.
            // TODO Wtf does this do?
            //  My best guess is that descriptor sets can have multile bindings to different sets of buffers.
            //  original comment said 'write to the first, and only binding.'
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.descriptorCount = storageBuffers; // update all descriptors in set.
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
            writeDescriptorSet.pBufferInfo = bindings;
        }
        

        // perform the update of the descriptor set.
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
    }

    // Read file into array of bytes, and cast to uint32_t*, then return.
    // The data has been padded, so that it fits into an array uint32_t.
    uint32_t* readFile(uint32_t& length, const char* filename) {

        FILE* fp = fopen(filename, "rb");
        if (fp == NULL) {
            printf("Could not find or open file: %s\n", filename);
        }

        // get file size.
        fseek(fp, 0, SEEK_END);
        long filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        long filesizepadded = long(ceil(filesize / 4.0)) * 4;

        // read file contents.
        char *str = new char[filesizepadded];
        fread(str, filesize, sizeof(char), fp);
        fclose(fp);

        // data padding. 
        for (int i = filesize; i < filesizepadded; i++) {
            str[i] = 0;
        }

        length = filesizepadded;
        return (uint32_t *)str;
    }

    // Creates compute pipeline
    void createComputePipeline(
        VkDevice& device,
        char* shaderFile,
        VkShaderModule* computeShaderModule,
        VkDescriptorSetLayout* descriptorSetLayout,
        VkPipelineLayout* pipelineLayout,
        VkPipeline* pipeline,
        float const* pushConstants,
        uint32_t numPushConstants
    ) {
        // Creates shader module (just a wrapper around our shader)
        VkShaderModuleCreateInfo createInfo = {};
        {
            uint32_t filelength;
            //uint32_t* code = readFile(filelength, "shaders/comp.spv");
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pCode = readFile(filelength, shaderFile);
            createInfo.codeSize = filelength;
            //delete[] code;
        }

        VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, computeShaderModule));

        // A compute pipeline is very simple compared to a graphics pipeline.
        // It only consists of a single stage with a compute shader.

        VkPushConstantRange push_constant;
        {
            push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            push_constant.offset = 0;
            push_constant.size = numPushConstants * sizeof(float);
        }

        // The pipeline layout allows the pipeline to access descriptor sets. 
        // So we just specify the descriptor set layout we created earlier.
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        {
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 1; // 1 shader
            pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayout; // Descriptor set
            pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pipelineLayoutCreateInfo.pPushConstantRanges = &push_constant;
        }
        
        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, pipelineLayout));

        // Set our pipeline options
        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        {
            // We specify the compute shader stage, and it's entry point(main).
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
            {
                shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; // Shader type
                shaderStageCreateInfo.module = *computeShaderModule; // Shader module
                shaderStageCreateInfo.pName = "main"; // Shader entry point
            }
            // We set our pipeline options
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.stage = shaderStageCreateInfo; // Shader stage info
            pipelineCreateInfo.layout = *pipelineLayout;
        }
        

        // Create compute pipeline
        VK_CHECK_RESULT(vkCreateComputePipelines(
            device, VK_NULL_HANDLE,
            1, &pipelineCreateInfo,
            NULL, pipeline));
    }
    
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
        int const* dims, // [x,y,z],
        int const* dimLengths // [local_size_x, local_size_y, local_size_z]
    ) {
        // Creates command pool
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        {
            commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            commandPoolCreateInfo.flags = 0;
            // Sets queue family
            commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        }
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, commandPool));

        //  Allocates command buffer
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        {
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = *commandPool; // Pool to allocate from
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = 1; // Allocates 1 command buffer. 
        }
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffer)); // allocate command buffer.

        // Allocated command buffer options
        VkCommandBufferBeginInfo beginInfo = {};
        {
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            // Buffer only submitted once
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        // Start recording commands
        VK_CHECK_RESULT(vkBeginCommandBuffer(*commandBuffer, &beginInfo));

        // Binds pipeline (our functions)
        vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        // Binds descriptor set (our data)
        vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
        
        // Sets push constants
        vkCmdPushConstants(*commandBuffer, pipelineLayout,VK_SHADER_STAGE_COMPUTE_BIT,0, numPushConstants*sizeof(float), pushConstants);

        std::cout << "workgroups: " << '(' <<
            (uint32_t)ceil(dims[0] / (float)dimLengths[0]) << ',' <<
            (uint32_t)ceil(dims[1] / (float)dimLengths[1]) << ',' <<
            (uint32_t)ceil(dims[2] / (float)dimLengths[2]) << ')' << 
            std::endl << std::endl;

        // Sets invocations
        vkCmdDispatch(
            *commandBuffer,
            (uint32_t) ceil(dims[0] / (float) dimLengths[0]),
            (uint32_t) ceil(dims[1] / (float) dimLengths[1]),
            (uint32_t) ceil(dims[2] / (float) dimLengths[2])
        );

        // End recording commands
        VK_CHECK_RESULT(vkEndCommandBuffer(*commandBuffer));
    }
    
    // Submits command buffer to queue for execution
    void runCommandBuffer(
        VkCommandBuffer* commandBuffer,
        VkDevice& device,
        VkQueue& queue
    ) {
        VkSubmitInfo submitInfo = {};
        {
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            // submit 1 command buffer
            submitInfo.commandBufferCount = 1;
            // pointer to array of command buffers to submit
            submitInfo.pCommandBuffers = commandBuffer;
        }

        // Creates fence (so we can await for command buffer to finish)
        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        {
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = 0; // TODO Add explanation of this
        }
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

        // Submit command buffer with fence
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for fence to signal (which it does when command buffer has finished)
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

        // Destructs fence
        vkDestroyFence(device, fence, NULL);
    }
    /*
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    uint32_t queueFamilyIndex;
    VkDevice device;
    VkQueue queue;

    VkBuffer* buffers;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkShaderModule computeShaderModule;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    */

    void printOutput(
        VkDevice& device,
        VkDeviceMemory& bufferMemory,
        uint32_t size
    ) {
        void* data = nullptr;
        vkMapMemory(device, bufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
        float* actualData = (float*)data;
        std::cout << "out:" << std::endl;
        std::cout << '\t';
        for (int i = 0; i < size; ++i) {
            std::cout << actualData[i] << ' ';
        }
        std::cout << std::endl;
        vkUnmapMemory(device, bufferMemory);
    }

    // Cleans up - Destructs everything
    void cleanup() {
        // TODO These top 2 probably wrong
        vkFreeMemory(device, *bufferMemories, NULL);
        vkDestroyBuffer(device, *buffers, NULL);
        vkDestroyShaderModule(device, computeShaderModule, NULL);
        vkDestroyDescriptorPool(device, descriptorPool, NULL);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);
        vkDestroyPipeline(device, pipeline, NULL);
        vkDestroyCommandPool(device, commandPool, NULL);	
        vkDestroyDevice(device, NULL);
        vkDestroyInstance(instance, NULL);		
    }
};

int main() {
    ComputeApplication app;

    try {
        uint32_t size = 10;
        float** data = new float*[1];
        data[0] = new float[size]{ 1,2,3,4,5,5,4,3,2,1 };
        
        app.run(
            new uint32_t[1]{ size }, // Buffer sizes
            1, //  Number of buffers
            data, // Buffer data
            new float[1]{ 7 }, // Push constants
            1, // Number of push constants
            new int[3]{ 10,1,1 }, // Invocations
            new int[3]{ 1024,1,1 } // Workgroup sizes
        );
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
