#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <unordered_set>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

class HelloTriangleApplication {
 public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
  void initVulkan() {
    createInstance();
    setupDebugMessenger();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }
  void cleanup() {
    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
  }

  void createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
    hasGflwRequiredInstanceExtensions();
  }

  void hasGflwRequiredInstanceExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());

    std::cout << "available extensions:" << std::endl;
    std::unordered_set<std::string> available;
    for (const auto& extension : extensions) {
      std::cout << "\t" << extension.extensionName << std::endl;
      available.insert(extension.extensionName);
    }

    std::cout << "required extensions:" << std::endl;
    auto requiredExtensions = getRequiredExtensions();
    for (const auto& required : requiredExtensions) {
      std::cout << "\t" << required << std::endl;
      if (available.find(required) == available.end()) {
        throw std::runtime_error("Missing required glfw extension");
      }
    }
  }

  bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
      bool layerFound = false;
      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          std::cout << "Found validation layer: " << layerName << std::endl;
          layerFound = true;
          break;
        }
      }
      if (!layerFound) {
        return false;
      }
    }
    return true;
  }

  std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
  }

  void setupDebugMessenger() {
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                     &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }

  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
      const VkAllocationCallbacks* pAllocator,
      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance, debugMessenger, pAllocator);
    }
  }

  // void pickPhysicalDevice() {
  //   uint32_t deviceCount = 0;
  //   vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  //   if (deviceCount == 0) {
  //     throw std::runtime_error("failed to find GPUs with Vulkan support!");
  //   }
  //   std::cout << "Device count: " << deviceCount << std::endl;
  //   std::vector<VkPhysicalDevice> devices(deviceCount);
  //   vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  //   for (const auto& device : devices) {
  //     if (isDeviceSuitable(device)) {
  //       physicalDevice = device;
  //       break;
  //     }
  //   }

  //   if (physicalDevice == VK_NULL_HANDLE) {
  //     throw std::runtime_error("failed to find a suitable GPU!");
  //   }

  //   vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  //   std::cout << "physical device: " << properties.deviceName << std::endl;
  // }

  // bool isDeviceSuitable(VkPhysicalDevice device) {
  //   QueueFamilyIndices indices = findQueueFamilies(device);

  //   bool extensionsSupported = checkDeviceExtensionSupport(device);

  //   bool swapChainAdequate = false;
  //   if (extensionsSupported) {
  //     SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
  //     swapChainAdequate = !swapChainSupport.formats.empty() &&
  //                         !swapChainSupport.presentModes.empty();
  //   }

  //   VkPhysicalDeviceFeatures supportedFeatures;
  //   vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  //   return indices.isComplete() && extensionsSupported && swapChainAdequate &&
  //          supportedFeatures.samplerAnisotropy;
  // }

  // QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
  //   QueueFamilyIndices indices;

  //   uint32_t queueFamilyCount = 0;
  //   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
  //                                            nullptr);

  //   std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  //   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
  //                                            queueFamilies.data());

  //   int i = 0;
  //   for (const auto& queueFamily : queueFamilies) {
  //     if (queueFamily.queueCount > 0 &&
  //         queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
  //       indices.graphicsFamily = i;
  //       indices.graphicsFamilyHasValue = true;
  //     }
  //     VkBool32 presentSupport = false;
  //     vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_,
  //                                          &presentSupport);
  //     if (queueFamily.queueCount > 0 && presentSupport) {
  //       indices.presentFamily = i;
  //       indices.presentFamilyHasValue = true;
  //     }
  //     if (indices.isComplete()) {
  //       break;
  //     }

  //     i++;
  //   }

  //   return indices;
  // }

  // SwapChainSupportDetails querySwapChainSupport(
  //     VkPhysicalDevice device) {
  //   SwapChainSupportDetails details;
  //   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_,
  //                                             &details.capabilities);

  //   uint32_t formatCount;
  //   vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount,
  //                                        nullptr);

  //   if (formatCount != 0) {
  //     details.formats.resize(formatCount);
  //     vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount,
  //                                          details.formats.data());
  //   }

  //   uint32_t presentModeCount;
  //   vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_,
  //                                             &presentModeCount, nullptr);

  //   if (presentModeCount != 0) {
  //     details.presentModes.resize(presentModeCount);
  //     vkGetPhysicalDeviceSurfacePresentModesKHR(
  //         device, surface_, &presentModeCount, details.presentModes.data());
  //   }
  //   return details;
  // }

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  GLFWwindow* window;
};

int main() {
  HelloTriangleApplication app;
  try {
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
