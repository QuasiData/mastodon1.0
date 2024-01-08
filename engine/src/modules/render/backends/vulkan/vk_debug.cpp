// ReSharper disable CppClangTidyClangDiagnosticCastFunctionTypeStrict
// ReSharper disable CppInconsistentNaming
#include "vk_debug.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace mas::gfx::vulkan::debug
{
namespace
{
VkDebugUtilsMessengerEXT debug_utils_messenger{ nullptr };

PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT = nullptr;
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT = nullptr;
PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT = nullptr;
PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTagEXT = nullptr;
PFN_vkSubmitDebugUtilsMessageEXT vkSubmitDebugUtilsMessageEXT = nullptr;

#pragma warning( push )
#pragma warning( disable : 4100 )
VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data)
{
    // Select prefix depending on flags passed to the callback
    std::string prefix;

    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        prefix = "VERBOSE: ";
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        prefix = "INFO: ";
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        prefix = "WARNING: ";
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        prefix = "ERROR: ";
    }

    // Display message to default output (console/logcat)
    std::stringstream debug_message;
    debug_message << prefix << "[" << p_callback_data->messageIdNumber << "][" << p_callback_data->pMessageIdName << "] : " << p_callback_data->pMessage;

    if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        std::cerr << debug_message.str() << "\n";
    }
    else
    {
        std::cout << debug_message.str() << "\n";
    }
    fflush(stdout); // NOLINT(cert-err33-c)

    return VK_FALSE;
}

void setup_debugging(const VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT call_back)
{
    vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_ci{};
    debug_utils_messenger_ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    debug_utils_messenger_ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    debug_utils_messenger_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_utils_messenger_ci.pfnUserCallback = debug_utils_messenger_callback;
    const VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debug_utils_messenger_ci, nullptr, &debug_utils_messenger);
    assert(result == VK_SUCCESS);
}
#pragma warning( pop )

void free_debug_callback(const VkInstance instance)
{
    if (debug_utils_messenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(instance, debug_utils_messenger, nullptr);
    }
}
}

void populate_ci(VkDebugUtilsMessengerCreateInfoEXT& ci)
{
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    ci.pfnUserCallback = debug_utils_messenger_callback;
}

void init(const VkInstance instance)
{
    constexpr VkDebugReportFlagsEXT flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    setup_debugging(instance, flags, nullptr);
}

void shutdown(const VkInstance instance)
{
    free_debug_callback(instance);
}
}