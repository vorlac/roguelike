#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include <imgui.h>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <enet/enet.h>
#include <vulkan/vulkan.h>

#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
SDL_C_LIB_END

typedef struct
{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    VmaAllocator allocator;
    VkSurfaceKHR surface;
} VulkanContext;

typedef struct
{
    SDL_Window* window;
    VulkanContext context;
    ImGuiContext* imgui;
    bool shouldClose;
} App;

void createVulkanContext(VulkanContext* ctx, Uint32 apiVersion)
{
    SDL_Vulkan_LoadLibrary(NULL);
    volkInitializeCustom((PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr());

    Uint32 extensionsCount;
    const char* const* requiredExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionsCount);

    vkCreateInstance(
        &(VkInstanceCreateInfo){
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .enabledExtensionCount = extensionsCount,
            .ppEnabledExtensionNames = requiredExtensions,
            .pApplicationInfo =
                &(VkApplicationInfo){
                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .apiVersion = apiVersion,
                },

        },
        NULL, &ctx->instance);

    volkLoadInstanceOnly(ctx->instance);
    vkEnumeratePhysicalDevices(ctx->instance, &(Uint32){ 1 }, &ctx->physicalDevice);

    vkCreateDevice(ctx->physicalDevice,
                   &(VkDeviceCreateInfo){
                       .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                       .enabledExtensionCount = 1,
                       .ppEnabledExtensionNames =
                           (const char*[]){
                               VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                           },
                       .queueCreateInfoCount = 1,
                       .pQueueCreateInfos =
                           &(VkDeviceQueueCreateInfo){
                               .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                               .queueFamilyIndex = 0,
                               .queueCount = 1,
                               .pQueuePriorities = (float[]){ 1.0f },
                           },
                   },
                   NULL, &ctx->device);
    volkLoadDevice(ctx->device);

    VkQueue queue;
    vkGetDeviceQueue(ctx->device, 0, 0, &queue);

    vmaCreateAllocator(
        &(VmaAllocatorCreateInfo){
            .physicalDevice = ctx->physicalDevice,
            .device = ctx->device,
            .vulkanApiVersion = apiVersion,
            .instance = ctx->instance,
            .pVulkanFunctions =
                &(VmaVulkanFunctions){
                    .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
                    .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
                    .vkAllocateMemory = vkAllocateMemory,
                    .vkFreeMemory = vkFreeMemory,
                    .vkMapMemory = vkMapMemory,
                    .vkUnmapMemory = vkUnmapMemory,
                    .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
                    .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
                    .vkBindBufferMemory = vkBindBufferMemory,
                    .vkBindImageMemory = vkBindImageMemory,
                    .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
                    .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
                    .vkCreateBuffer = vkCreateBuffer,
                    .vkDestroyBuffer = vkDestroyBuffer,
                    .vkCreateImage = vkCreateImage,
                    .vkDestroyImage = vkDestroyImage,
                    .vkCmdCopyBuffer = vkCmdCopyBuffer,
                    .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2,
                    .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2,
                    .vkBindBufferMemory2KHR = vkBindBufferMemory2,
                    .vkBindImageMemory2KHR = vkBindImageMemory2,
                    .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
                },
        },
        &ctx->allocator);
}

void bindWindow(VulkanContext* context, SDL_Window* window)
{
    if (context->surface)
        return;
    SDL_Vulkan_CreateSurface(window, context->instance, &context->surface);
}

void unbindWindow(VulkanContext* context)
{
    if (!context->surface)
        return;
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    context->surface = VK_NULL_HANDLE;
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    TTF_Init();
    enet_initialize();

    App app;
    app.window = SDL_CreateWindow("Hello World", 800, 600,
                                  SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);

    createVulkanContext(&app.context, VK_API_VERSION_1_2);

    ImGuiContext* context = igCreateContext(NULL);

#ifndef __ANDROID__
    bindWindow(&app.context, app.window);
#endif

    SDL_ShowWindow(app.window);

    while (!app.shouldClose)
    {
        for (SDL_Event event; SDL_PollEvent(&event);)
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    app.shouldClose = true;
                    break;
                case SDL_EVENT_DID_ENTER_FOREGROUND:
                    bindWindow(&app.context, app.window);
                    break;
                case SDL_EVENT_WILL_ENTER_BACKGROUND:
                    unbindWindow(&app.context);
                    break;
            }
        }
    }

    vkDeviceWaitIdle(app.context.device);

    return 0;
}
