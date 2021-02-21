#include "Example.hpp"

#include <filesystem>

// Gets Vulkan instance
void Utility::createInstance(VkInstance& instance) {
    std::vector<char const*> enabledLayers;
    std::vector<char const*> enabledExtensions;

    if (enableValidationLayers.has_value()) {
        // Gets number of supported layers
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        // Gets all supported layers
        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        // Check 'VK_LAYER_KHRONOS_validation' is among supported layers
        auto layer_itr = std::find_if(layerProperties.begin(), layerProperties.end(),
            [](VkLayerProperties& prop) {
                return (strcmp(enableValidationLayers.value(), prop.layerName) == 0);
            }
        );
        // If not, throw error
        if (layer_itr == layerProperties.end()) {
            throw std::runtime_error("Validation layer not supported\n");
        }
        // Else, push to layers
        enabledLayers.push_back(enableValidationLayers.value());

        // We need to enable the extension VK_EXT_DEBUG_REPORT,
        //  to print the warnings emitted by the validation layer.

        // Gets number of supported extensions
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        // Gets all supported extensions
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

        // Check `VK_EXT_DEBUG_REPORT` is among supported extensions
        bool debug_report = false;
        for(VkExtensionProperties& prop: extensionProperties) {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) { 
                debug_report=true; 
                break;
            }
        }
        // If not, throw error
        if (!debug_report) {
            throw std::runtime_error(
                "Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n"
            );
        }
        // Else, push to extensions
        enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_1 // Vulkan version
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
        .ppEnabledLayerNames = enabledLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
        .ppEnabledExtensionNames = enabledExtensions.data()
    };

    // Creates instance
    // TODO Do we need these `pragma`s?
    // #pragma warning(disable : 26812) // Removes enum scoping warnings from Vulkan
    VK_CHECK_RESULT(vkCreateInstance(
        &createInfo,
        nullptr,
        &instance)
    );
    // #pragma warning(default : 26812)
}

// Gets physical device
void Utility::getPhysicalDevice(VkInstance const& instance, VkPhysicalDevice& physicalDevice) {
    // Gets number of physical devices
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    // Asserts a system has a device
    assert(deviceCount != 0);

    // Gets physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Picks 1st
    physicalDevice = devices[0];
}

// Gets index of 1st queue family which supports compute
uint32_t Utility::getComputeQueueFamilyIndex(VkPhysicalDevice const& physicalDevice) {
    // Gets number of queue families
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyCount, nullptr
    );

    // Gets queue families
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyCount, queueFamilies.data()
    );

    // Finds 1st queue family which supports compute
    auto itr = std::find_if(queueFamilies.begin(), queueFamilies.end(),
        [](VkQueueFamilyProperties& props) {
            return (props.queueFlags & VK_QUEUE_COMPUTE_BIT);
        }
    );
    if (itr == queueFamilies.end()) {
        throw std::runtime_error("No compute queue family");
    }
    return static_cast<uint32_t>(std::distance(queueFamilies.begin(), itr));
}

// Creates logical device
void Utility::createDevice(
    VkPhysicalDevice const& physicalDevice,
    uint32_t& queueFamilyIndex,
    VkDevice& device,
    VkQueue& queue
) {
    // Find queue family with compute capability.
    queueFamilyIndex = getComputeQueueFamilyIndex(physicalDevice);
    // Device queue info
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount = 1, // create one queue in this family. We don't need more.
        .pQueuePriorities = new float(1)
    };
    // Device info
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo
    };

    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)); // create logical device.

    // Get handle to queue 0 in `queueFamilyIndex` queue family
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
}

// Finds memory type with given properties
uint32_t Utility::findMemoryType(
    VkPhysicalDevice const& physicalDevice, 
    uint32_t const memoryTypeBits, 
    VkMemoryPropertyFlags const properties
) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

   // Iterate through memory types available on our physical device
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        // If our resource supports a memory type, and
        //  a memory type contains all required properties, then
        //  return the index of this memory type
        if (
            // Check resource (buffer) supports this memory type
            (memoryTypeBits & (1 << i)) &&
            // Check all required properties are supported by this memory type.
            //  `x&y==y` <=> `x contains y` <=> `All 1 bits in y are 1 bits in x`
            //  <=> All features of `properties` are in `memoryTypes[i].propertyFlags`
            ((memoryProperties.memoryTypes[i].propertyFlags & properties)==properties)
        ) {
            return i;
        }
    }
    return -1;
}

// Creates buffer
void Utility::createBuffers(
    VkPhysicalDevice const& physicalDevice,
    VkDevice const& device,
    uint32_t const numBuffers,
    uint32_t const* bufferSize,
    VkBuffer*& buffer,
    VkDeviceMemory*& bufferMemory
) {
    buffer = new VkBuffer[numBuffers];
    bufferMemory = new VkDeviceMemory[numBuffers];
    for(uint32_t i=0;i<numBuffers;++i) {
        // Creates buffer
        Utility::createBuffer(physicalDevice, device, bufferSize[i], &buffer[i], &bufferMemory[i]);
    }
}

// Creates buffer
void Utility::createBuffer(
    VkPhysicalDevice const& physicalDevice,
    VkDevice const& device,
    uint32_t const size,
    VkBuffer * const buffer,
    VkDeviceMemory * const bufferMemory
) {
    // Buffer info
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        // buffer size in bytes.
        .size = sizeof(float)*size,
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

// Fills buffers with data
// Must be after `createBuffers` but before `createComputePipeline`
void Utility::fillBuffer(
    VkDevice const& device,
    VkDeviceMemory& bufferMemory,
    float*& bufferData, 
    uint32_t const bufferSize
) {
    void* data = nullptr;
    // Maps buffer memory into RAM
    vkMapMemory(device, bufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
    // Fills buffer memory
    memcpy(data, bufferData, static_cast<uint32_t>(bufferSize * sizeof(float)));
    // Un-maps buffer memory from RAM to device memory
    vkUnmapMemory(device, bufferMemory);
}

// Creates descriptor set layout
void Utility::createDescriptorSetLayout(
    VkDevice const& device,
    VkDescriptorSetLayout* descriptorSetLayout,
    uint32_t const numBuffers
) {
    VkDescriptorSetLayoutBinding* binding = new VkDescriptorSetLayoutBinding[numBuffers];
    for(uint32_t i=0;i<numBuffers;++i){
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
        .bindingCount = numBuffers,
        // array of `VkDescriptorSetLayoutBinding`s
        .pBindings = binding
    };
    
    // Create the descriptor set layout. 
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(
        device, &descriptorSetLayoutCreateInfo, nullptr, descriptorSetLayout
    ));
}

// Creates descriptor set
void Utility::createDescriptorSet(
    VkDevice const& device,
    VkDescriptorPool* descriptorPool,
    VkDescriptorSetLayout* descriptorSetLayout,
    uint32_t const numBuffers,
    VkBuffer*& buffer,
    VkDescriptorSet& descriptorSet
) {
    // Descriptor type and number
    VkDescriptorPoolSize descriptorPoolSize = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = numBuffers // Number of descriptors
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
    VkDescriptorBufferInfo* binding = new VkDescriptorBufferInfo[numBuffers];
    for(uint32_t i=0;i<numBuffers;++i){
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
        .descriptorCount = numBuffers,
        // buffer type.
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        // respective buffer.
        .pBufferInfo = binding
    };
    
    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

// Reads shader file
std::pair<uint32_t,uint32_t*> Utility::readShader(char const* filename) {
    // std::string path = "../../../";
    // std::cout << "paths:" << std::endl;
    // for (const auto & entry : std::filesystem::directory_iterator(path)) {
    //     std::cout << entry.path() << std::endl;
    // }
    
    // Open file
    FILE* fp = fopen(filename, "rb");
    if (fp == nullptr) {
        throw std::runtime_error("Could not open shader");
    }

    // Get file size.
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    // Get file size ceiled to multiple of 4 bytes
    fseek(fp, 0, SEEK_SET);
    long filesizepadded = long(ceil(filesize / 4.0)) * 4;

    // Read file
    char *str = new char[filesizepadded];
    fread(str, filesizepadded, sizeof(char), fp);

    // Close file
    fclose(fp);

    // zeros last 0 to 3 bytes
    for (int i = filesize; i < filesizepadded; i++) {
        str[i] = 0;
    }

    return std::make_pair(filesizepadded,(uint32_t *)str);
}

// Runs command buffer
void Utility::runCommandBuffer(
    VkCommandBuffer* commandBuffer,
    VkDevice const& device,
    VkQueue const& queue
) {
    // Creates fence (so we can await for command buffer to finish)
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

    // Command buffer submit info
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        // submit 1 command buffer
        .commandBufferCount = 1,
        // pointer to array of command buffers to submit
        .pCommandBuffers = commandBuffer
    };
    // Submit command buffer with fence
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

    // Wait for fence to signal (which it does when command buffer has finished)
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    // Destructs fence
    vkDestroyFence(device, fence, nullptr);
}

// Maps buffer to CPU memory
void* Utility::map(
    VkDevice& device,
    VkDeviceMemory& bufferMemory
) {
    void* data = nullptr;
    vkMapMemory(device, bufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
    return data;
}