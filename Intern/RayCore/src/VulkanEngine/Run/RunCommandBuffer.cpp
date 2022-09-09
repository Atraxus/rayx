#include "VulkanEngine/VulkanEngine.h"

namespace RAYX {

void VulkanEngine::runCommandBuffer() {
    RAYX_PROFILE_FUNCTION();
    /*
    Now we shall finally submit the recorded command buffer to a queue.
    */

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;  // submit a single command buffer
    submitInfo.pCommandBuffers =
        &m_CommandBuffer;  // the command buffer to submit.

    /*
        We create a fence.
    */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &fence));

    /*
    We submit the command buffer on the queue, at the same time giving a
    fence. (Fences are like interrupts and used for async computations)
    */
    VK_CHECK_RESULT(vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, fence));
    /*
    The command will not have finished executing until the fence is
    signaled. So we wait here. Directly afer this, we read our buffer
    from the GPU. Fences give us a hint that the Command in the Queue is
    actually done executing.
    */
    VK_CHECK_RESULT(
        vkWaitForFences(m_Device, 1, &fence, VK_TRUE, 1000000000000000));

    vkDestroyFence(m_Device, fence, nullptr);
}

}
