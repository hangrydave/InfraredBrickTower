// Dear ImGui: standalone example application for Glfw + Vulkan
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <thread>

#include <LASM.h>
#include <imfilebrowser.h>
#include <TowerController.h>
#include <VLL.h>
#include <PBrick.h>

#if defined(WIN64)
#include "WinUsbTowerInterface.h"
#elif defined(__linux)
#include "LinuxUSBTowerInterface.h"
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

static VkAllocationCallbacks*   g_Allocator = NULL;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int                      g_MinImageCount = 2;
static bool                     g_SwapChainRebuild = false;

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

#ifdef IMGUI_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

static void SetupVulkan(const char** extensions, uint32_t extensions_count)
{
    VkResult err;

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.enabledExtensionCount = extensions_count;
        create_info.ppEnabledExtensionNames = extensions;
#ifdef IMGUI_VULKAN_DEBUG_REPORT
        // Enabling validation layers
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;

        // Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
        const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
        memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
        extensions_ext[extensions_count] = "VK_EXT_debug_report";
        create_info.enabledExtensionCount = extensions_count + 1;
        create_info.ppEnabledExtensionNames = extensions_ext;

        // Create Vulkan Instance
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);
        free(extensions_ext);

        // Get the function pointer (required for any extensions)
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

        // Setup the debug report callback
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = NULL;
        err = vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
        check_vk_result(err);
#else
        // Create Vulkan Instance without any debug feature
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);
        IM_UNUSED(g_DebugReport);
#endif
    }

    // Select GPU
    {
        uint32_t gpu_count;
        err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
        check_vk_result(err);
        IM_ASSERT(gpu_count > 0);

        VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
        err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);
        check_vk_result(err);

        // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
        // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
        // dedicated GPUs) is out of scope of this sample.
        int use_gpu = 0;
        for (int i = 0; i < (int)gpu_count; i++)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(gpus[i], &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                use_gpu = i;
                break;
            }
        }

        g_PhysicalDevice = gpus[use_gpu];
        free(gpus);
    }

    // Select graphics queue family
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
        VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
        for (uint32_t i = 0; i < count; i++)
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                g_QueueFamily = i;
                break;
            }
        free(queues);
        IM_ASSERT(g_QueueFamily != (uint32_t)-1);
    }

    // Create Logical Device (with 1 queue)
    {
        int device_extension_count = 1;
        const char* device_extensions[] = { "VK_KHR_swapchain" };
        const float queue_priority[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = g_QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = device_extension_count;
        create_info.ppEnabledExtensionNames = device_extensions;
        err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
        check_vk_result(err);
        vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // Create Descriptor Pool
    {
        VkDescriptorPoolSize pool_sizes[] =
                {
                        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
}

static void CleanupVulkan()
{
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    if (g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#define BYTE unsigned char
#define WORD unsigned short

static bool programIsDone = false;

static ImGuiViewport* mainViewport;
static ImGui::FileBrowser fileDialog;

static unsigned long towerLengthWritten = 0;

static Tower::CommMode currentTowerCommMode;

static LASM::CommandData lasmCommand;
struct RCXRemoteData
{
    bool motorAFwd,
            motorBFwd,
            motorCFwd,
            motorABwd,
            motorBBwd,
            motorCBwd = false;

    int message1,
            message2,
            message3 = 0;

    int program1,
            program2,
            program3,
            program4,
            program5 = 0;

    int stop = 0;
    int sound = 0;

    std::string* downloadFilePath;

    WORD request = 0;
} static rcxRemoteData;

struct VLLData
{
    int beep1Immediate,
            beep2Immediate,
            beep3Immediate,
            beep4Immediate,
            beep5Immediate = 0;
    BYTE beep1ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_1_IMMEDIATE };
    BYTE beep2ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_2_IMMEDIATE };
    BYTE beep3ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_3_IMMEDIATE };
    BYTE beep4ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_4_IMMEDIATE };
    BYTE beep5ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_5_IMMEDIATE };

    bool forwardImmediate = false;
    bool backwardImmediate = false;
    BYTE fwdImmediateBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_IMMEDIATE };
    BYTE bwdImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_IMMEDIATE };

    int beep1Program,
            beep2Program,
            beep3Program,
            beep4Program,
            beep5Program = 0;
    BYTE beep1ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_1_PROGRAM };
    BYTE beep2ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_2_PROGRAM };
    BYTE beep3ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_3_PROGRAM };
    BYTE beep4ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_4_PROGRAM };
    BYTE beep5ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_5_PROGRAM };

    int forwardHalf,
            forwardOne,
            forwardTwo,
            forwardFive = 0;
    BYTE forwardHalfBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_HALF };
    BYTE forwardOneByte[VLL_PACKET_LENGTH]{ VLL_FORWARD_ONE };
    BYTE forwardTwoBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_TWO };
    BYTE forwardFiveBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_FIVE };

    int backwardHalf,
            backwardOne,
            backwardTwo,
            backwardFive = 0;
    BYTE backwardHalfBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_HALF };
    BYTE backwardOneByte[VLL_PACKET_LENGTH]{ VLL_BACKWARD_ONE };
    BYTE backwardTwoBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_TWO };
    BYTE backwardFiveBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_FIVE };

    int waitLight,
            seekLight,
            code,
            keepAlive = 0;
    BYTE waitLightBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
    BYTE seekLightBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
    BYTE codeBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
    BYTE keepAliveBytes[VLL_PACKET_LENGTH]{ VLL_STOP };

    int run = 0;
    BYTE runBytes[VLL_PACKET_LENGTH]{ VLL_RUN };

    int stop = 0;
    BYTE stopBytes[VLL_PACKET_LENGTH]{ VLL_STOP };

    int deleteProgram = 0;
    BYTE deleteBytes[VLL_PACKET_LENGTH]{ VLL_DELETE_PROGRAM };
} static vllData;

void SendVLL(BYTE* data, Tower::RequestData* towerData) {
    if (currentTowerCommMode != Tower::CommMode::VLL) {
        Tower::SetCommMode(Tower::CommMode::VLL, towerData);
    }

    Tower::WriteData(data, VLL_PACKET_LENGTH, towerLengthWritten, towerData);
}

void RunTowerThread()
{
    HostTowerCommInterface* usbTowerInterface;
#if defined(WIN64)
    bool gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
    if (!gotInterface)
    {
        printf("Error getting WinUSB interface!\n");
        programIsDone = true;
        return;
    }
#elif defined(__linux)
    bool gotInterface = OpenLinuxUSBTowerInterface(usbTowerInterface);
    if (!gotInterface)
    {
        printf("Error getting Linux USB interface!\n");
        programIsDone = true;
        return;
    }
#endif

    Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);

    // get into IR mode right off of the bat
    currentTowerCommMode = Tower::CommMode::IR;
    Tower::SetCommMode(currentTowerCommMode, towerData);

    while (!programIsDone)
    {
        // RCX
        {
            // remote
            rcxRemoteData.request = 0;

            if (rcxRemoteData.message1-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PB_MESSAGE_1;
            if (rcxRemoteData.message2-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PB_MESSAGE_2;
            if (rcxRemoteData.message3-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PB_MESSAGE_3;

            if (rcxRemoteData.motorAFwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_A_FORWARDS;
            if (rcxRemoteData.motorABwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_A_BACKWARDS;

            if (rcxRemoteData.motorBFwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_B_FORWARDS;
            if (rcxRemoteData.motorBBwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_B_BACKWARDS;

            if (rcxRemoteData.motorCFwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_C_FORWARDS;
            if (rcxRemoteData.motorCBwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_C_BACKWARDS;

            if (rcxRemoteData.program1-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_1;
            if (rcxRemoteData.program2-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_2;
            if (rcxRemoteData.program3-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_3;
            if (rcxRemoteData.program4-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_4;
            if (rcxRemoteData.program5-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_5;

            if (rcxRemoteData.stop-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::STOP_PROGRAM_AND_MOTORS;
            if (rcxRemoteData.sound-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::REMOTE_SOUND;

            if (rcxRemoteData.request != 0)
            {
                if (currentTowerCommMode != Tower::CommMode::IR)
                    Tower::SetCommMode(Tower::CommMode::IR, towerData);

                LASM::Cmd_RemoteCommand(rcxRemoteData.request, lasmCommand);
                LASM::SendCommand(&lasmCommand, towerData, 0);
            }

            if (rcxRemoteData.downloadFilePath != nullptr)
            {
                if (currentTowerCommMode != Tower::CommMode::IR)
                    Tower::SetCommMode(Tower::CommMode::IR, towerData);

                RCX::DownloadProgram(rcxRemoteData.downloadFilePath->c_str(), 0, towerData);
                rcxRemoteData.downloadFilePath = nullptr;
            }
        }

        // VLL
        {
            // immediate
            // motor
            if (vllData.forwardImmediate)
                SendVLL(vllData.fwdImmediateBytes, towerData);
            if (vllData.backwardImmediate)
                SendVLL(vllData.bwdImmediateBytes, towerData);

            // sound
            if (vllData.beep1Immediate-- > 0)
                SendVLL(vllData.beep1ImmediateBytes, towerData);
            if (vllData.beep2Immediate-- > 0)
                SendVLL(vllData.beep2ImmediateBytes, towerData);
            if (vllData.beep3Immediate-- > 0)
                SendVLL(vllData.beep3ImmediateBytes, towerData);
            if (vllData.beep4Immediate-- > 0)
                SendVLL(vllData.beep4ImmediateBytes, towerData);
            if (vllData.beep5Immediate-- > 0)
                SendVLL(vllData.beep5ImmediateBytes, towerData);

            // program
            // sound
            if (vllData.beep1Program-- > 0)
                SendVLL(vllData.beep1ProgramBytes, towerData);
            if (vllData.beep2Program-- > 0)
                SendVLL(vllData.beep2ProgramBytes, towerData);
            if (vllData.beep3Program-- > 0)
                SendVLL(vllData.beep3ProgramBytes, towerData);
            if (vllData.beep4Program-- > 0)
                SendVLL(vllData.beep4ProgramBytes, towerData);
            if (vllData.beep5Program-- > 0)
                SendVLL(vllData.beep5ProgramBytes, towerData);

            // motor
            if (vllData.forwardHalf-- > 0)
                SendVLL(vllData.forwardHalfBytes, towerData);
            if (vllData.forwardOne-- > 0)
                SendVLL(vllData.forwardOneByte, towerData);
            if (vllData.forwardTwo-- > 0)
                SendVLL(vllData.forwardTwoBytes, towerData);
            if (vllData.forwardFive-- > 0)
                SendVLL(vllData.forwardFiveBytes, towerData);
            if (vllData.backwardHalf-- > 0)
                SendVLL(vllData.backwardHalfBytes, towerData);
            if (vllData.backwardOne-- > 0)
                SendVLL(vllData.backwardOneByte, towerData);
            if (vllData.backwardTwo-- > 0)
                SendVLL(vllData.backwardTwoBytes, towerData);
            if (vllData.backwardFive-- > 0)
                SendVLL(vllData.backwardFiveBytes, towerData);

            // preset programs
            if (vllData.waitLight-- > 0)
                SendVLL(vllData.waitLightBytes, towerData);
            if (vllData.seekLight-- > 0)
                SendVLL(vllData.seekLightBytes, towerData);
            if (vllData.code-- > 0)
                SendVLL(vllData.codeBytes, towerData);
            if (vllData.keepAlive-- > 0)
                SendVLL(vllData.keepAliveBytes, towerData);

            // etc functionality
            if (vllData.run-- > 0)
                SendVLL(vllData.runBytes, towerData);
            if (vllData.stop-- > 0)
                SendVLL(vllData.stopBytes, towerData);
            if (vllData.deleteProgram-- > 0)
                SendVLL(vllData.deleteBytes, towerData);
        }
    }

    delete towerData;
    delete usbTowerInterface;
}

void BuildMicroScoutRemote()
{
    ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 50, mainViewport->WorkPos.y + 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300));

    // VLL window
    ImGui::Begin("MicroScout Remote");

#define VLL_IMMEDIATE_MODE 0
#define VLL_PROGRAM_MODE 1
    static int commandMode = 0;
    ImGui::Text("Command mode");
    ImGui::RadioButton("immediate", &commandMode, VLL_IMMEDIATE_MODE); ImGui::SameLine();
    ImGui::RadioButton("program", &commandMode, VLL_PROGRAM_MODE);

    if (commandMode == VLL_IMMEDIATE_MODE)
    {
        // sounds
        ImGui::Separator();
        if (ImGui::Button("1"))
            vllData.beep1Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("2"))
            vllData.beep2Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("3"))
            vllData.beep3Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("4"))
            vllData.beep4Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("5"))
            vllData.beep5Immediate = 1;

        ImGui::SameLine();
        ImGui::Text("Sound");

        // motors
        ImGui::Separator();
        ImGui::Checkbox("motor forwards", &vllData.forwardImmediate);
        if (vllData.forwardImmediate)
            vllData.backwardImmediate = false;

        ImGui::SameLine();
        ImGui::Checkbox("motor backwards", &vllData.backwardImmediate);
        if (vllData.backwardImmediate)
            vllData.forwardImmediate = false;
    }
    else
    {
        // sounds
        ImGui::Separator();
        ImGui::Text("Sound");
        if (ImGui::Button("1"))
            vllData.beep1Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("2"))
            vllData.beep2Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("3"))
            vllData.beep3Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("4"))
            vllData.beep4Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("5"))
            vllData.beep5Program = 1;

        // motors
        // forward
        ImGui::Separator();
        ImGui::Text("Motor");
        if (ImGui::Button("fwd 0.5 sec"))
            vllData.forwardHalf = 1;
        ImGui::SameLine();
        if (ImGui::Button("fwd 1 sec"))
            vllData.forwardOne = 1;
        ImGui::SameLine();
        if (ImGui::Button("fwd 2 sec"))
            vllData.forwardTwo = 1;
        ImGui::SameLine();
        if (ImGui::Button("fwd 5 sec"))
            vllData.forwardFive = 1;

        // backward
        if (ImGui::Button("bwd 0.5 sec"))
            vllData.backwardHalf = 1;
        ImGui::SameLine();
        if (ImGui::Button("bwd 1 sec"))
            vllData.backwardOne = 1;
        ImGui::SameLine();
        if (ImGui::Button("bwd 2 sec"))
            vllData.backwardTwo = 1;
        ImGui::SameLine();
        if (ImGui::Button("bwd 5 sec"))
            vllData.backwardFive = 1;

        // program presets
        // wait seek code keep
        ImGui::Separator();
        ImGui::Text("Program presets");
        if (ImGui::Button("wait light"))
            vllData.waitLight = 1;
        ImGui::SameLine();
        if (ImGui::Button("seek light"))
            vllData.seekLight = 1;
        ImGui::SameLine();
        if (ImGui::Button("code"))
            vllData.code = 1;
        ImGui::SameLine();
        if (ImGui::Button("keep alive"))
            vllData.keepAlive = 1;

        // run/delete/stop
        ImGui::Separator();
        ImGui::Text("Management");
        if (ImGui::Button("run"))
            vllData.run = 1;
        ImGui::SameLine();
        if (ImGui::Button("stop"))
            vllData.stop = 1;
        ImGui::SameLine();
        if (ImGui::Button("delete program"))
            vllData.deleteProgram = 1;
    }

    ImGui::End();
}

void BuildRCXRemote()
{
    ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 50, mainViewport->WorkPos.y + 350), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300));

    // RCX remote
    ImGui::Begin("RCX Remote");

    // messages
    ImGui::Text("Message");
    if (ImGui::Button("Msg 1"))
        rcxRemoteData.message1 = true;
    ImGui::SameLine();
    if (ImGui::Button("Msg 2"))
        rcxRemoteData.message2 = true;
    ImGui::SameLine();
    if (ImGui::Button("Msg 3"))
        rcxRemoteData.message3 = true;

    // motors
    ImGui::Separator();
    ImGui::Text("Motors");

    if (ImGui::BeginTable("motorControlTable", 3))
    {
        ImVec2 buttonSize;
        buttonSize.x = -1;
        buttonSize.y = 0;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Checkbox("A Fwd", &rcxRemoteData.motorAFwd);
        if (rcxRemoteData.motorAFwd) rcxRemoteData.motorABwd = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("B Fwd", &rcxRemoteData.motorBFwd);
        if (rcxRemoteData.motorBFwd) rcxRemoteData.motorBBwd = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::Checkbox("C Fwd", &rcxRemoteData.motorCFwd);
        if (rcxRemoteData.motorCFwd) rcxRemoteData.motorCBwd = false;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Checkbox("A Bwd", &rcxRemoteData.motorABwd);
        if (rcxRemoteData.motorABwd) rcxRemoteData.motorAFwd = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("B Bwd", &rcxRemoteData.motorBBwd);
        if (rcxRemoteData.motorBBwd) rcxRemoteData.motorBFwd = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::Checkbox("C Bwd", &rcxRemoteData.motorCBwd);
        if (rcxRemoteData.motorCBwd) rcxRemoteData.motorCFwd = false;
    }
    ImGui::EndTable();

    // programs
    ImGui::Separator();
    ImGui::Text("Program");
    if (ImGui::Button("Prgm 1"))
        rcxRemoteData.program1 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 2"))
        rcxRemoteData.program2 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 3"))
        rcxRemoteData.program3 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 4"))
        rcxRemoteData.program4 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 5"))
        rcxRemoteData.program5 = 1;

    ImGui::Separator();
    if (ImGui::Button("Stop"))
        rcxRemoteData.stop = 1;
    ImGui::SameLine();
    if (ImGui::Button("Sound"))
        rcxRemoteData.sound = 1;

    ImGui::Separator();
    if (ImGui::Button("Download Program"))
        fileDialog.Open();

    ImGui::End();

    fileDialog.Display();
    if (fileDialog.HasSelected())
    {
        rcxRemoteData.downloadFilePath = new std::string(fileDialog.GetSelected().string());
        fileDialog.ClearSelected();
    }
}

int main(int, char**)
{
    // Setup GLFW window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+Vulkan example", NULL, NULL);

    // Setup Vulkan
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }
    uint32_t extensions_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    SetupVulkan(extensions, extensions_count);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
    check_vk_result(err);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

        err = vkResetCommandPool(g_Device, command_pool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(command_buffer, &begin_info);
        check_vk_result(err);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        err = vkEndCommandBuffer(command_buffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
        check_vk_result(err);

        err = vkDeviceWaitIdle(g_Device);
        check_vk_result(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Run the tower transmission things on a separate thread
    std::thread towerThread(RunTowerThread);

    mainViewport = ImGui::GetMainViewport();

    fileDialog.SetTitle("Select an RCX program");
    fileDialog.SetTypeFilters({ ".rcx" });

    // Main loop
    while (!glfwWindowShouldClose(window) && !programIsDone)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Resize swap chain?
        if (g_SwapChainRebuild)
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
                g_MainWindowData.FrameIndex = 0;
                g_SwapChainRebuild = false;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        BuildMicroScoutRemote();
        BuildRCXRemote();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }
    }

    // Cleanup
    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow();
    CleanupVulkan();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}