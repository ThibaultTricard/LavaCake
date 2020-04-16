#include "VulkanViewport.h"

namespace LavaCake {
	namespace Viewport {
		
		void SetViewportStateDynamically(VkCommandBuffer                 command_buffer,
			uint32_t                        first_viewport,
			std::vector<VkViewport> const & viewports) {
			vkCmdSetViewport(command_buffer, first_viewport, static_cast<uint32_t>(viewports.size()), viewports.data());
		}

		void SpecifyPipelineViewportAndScissorTestState(ViewportInfo const                & viewport_infos,
			VkPipelineViewportStateCreateInfo & viewport_state_create_info) {
			uint32_t viewport_count = static_cast<uint32_t>(viewport_infos.Viewports.size());
			uint32_t scissor_count = static_cast<uint32_t>(viewport_infos.Scissors.size());
			viewport_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,    // VkStructureType                      sType
				nullptr,                                                  // const void                         * pNext
				0,                                                        // VkPipelineViewportStateCreateFlags   flags
				viewport_count,                                           // uint32_t                             viewportCount
				viewport_infos.Viewports.data(),                          // const VkViewport                   * pViewports
				scissor_count,                                            // uint32_t                             scissorCount
				viewport_infos.Scissors.data()                            // const VkRect2D                     * pScissors
			};
		}

		void SetScissorStateDynamically(VkCommandBuffer               command_buffer,
			uint32_t                      first_scissor,
			std::vector<VkRect2D> const & scissors) {
			vkCmdSetScissor(command_buffer, first_scissor, static_cast<uint32_t>(scissors.size()), scissors.data());
		}

	}
}
