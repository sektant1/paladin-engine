/**
 * @file Context.h
 * @brief Vulkan instance + physical/logical device + queue selection.
 *
 * This is a *preview* scaffold. It is not wired into the engine's GL render
 * path; it exists so we can iterate on a Vulkan backend in parallel without
 * disturbing the working OpenGL pipeline. See engine/source/graphics/vulkan/
 * README considerations in CMakeLists (MONAD_VULKAN_PREVIEW option).
 */

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace mnd::vk
{

struct QueueFamilies
{
    uint32_t graphics = UINT32_MAX;
    uint32_t present  = UINT32_MAX;

    [[nodiscard]] bool IsComplete() const { return graphics != UINT32_MAX && present != UINT32_MAX; }
};

/**
 * @brief Owns the Vulkan instance, surface (tied to a GLFW window),
 *        physical + logical device, and graphics/present queues.
 *
 * Construction order matches typical Vulkan boot:
 *   1. CreateInstance      (+ optional debug messenger when validation layers requested)
 *   2. CreateSurface       (via glfwCreateWindowSurface)
 *   3. PickPhysicalDevice  (scores by queue family + swapchain support)
 *   4. CreateLogicalDevice (one queue from each unique family)
 */
class Context
{
 public:
    /// @param window Required — used to create a VkSurfaceKHR via GLFW.
    /// @param enableValidation Toggle VK_LAYER_KHRONOS_validation + debug messenger.
    Context(GLFWwindow *window, bool enableValidation);
    ~Context();

    Context(const Context &)            = delete;
    Context &operator=(const Context &) = delete;

    [[nodiscard]] VkInstance GetInstance() const { return m_instance; }

    [[nodiscard]] VkSurfaceKHR GetSurface() const { return m_surface; }

    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }

    [[nodiscard]] VkDevice GetDevice() const { return m_device; }

    [[nodiscard]] VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }

    [[nodiscard]] VkQueue GetPresentQueue() const { return m_presentQueue; }

    [[nodiscard]] QueueFamilies GetQueueFamilies() const { return m_queueFamilies; }

 private:
    void CreateInstance();
    void CreateDebugMessenger();
    void CreateSurface(GLFWwindow *window);
    void PickPhysicalDevice();
    void CreateLogicalDevice();

    bool                     m_enableValidation = false;
    VkInstance               m_instance         = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger   = VK_NULL_HANDLE;
    VkSurfaceKHR             m_surface          = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice   = VK_NULL_HANDLE;
    VkDevice                 m_device           = VK_NULL_HANDLE;
    VkQueue                  m_graphicsQueue    = VK_NULL_HANDLE;
    VkQueue                  m_presentQueue     = VK_NULL_HANDLE;
    QueueFamilies            m_queueFamilies;
};

}  // namespace mnd::vk
