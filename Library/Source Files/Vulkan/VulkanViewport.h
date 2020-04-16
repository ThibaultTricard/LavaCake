#ifndef VULKAN_VIEWPORT
#define VULKAN_VIEWPORT

#include "Common.h"

namespace LavaCake {
	namespace Viewport {
		struct ViewportInfo {
			std::vector<VkViewport>   Viewports;
			std::vector<VkRect2D>     Scissors;
		};
		void SetViewportStateDynamically(VkCommandBuffer                 command_buffer,
			uint32_t                        first_viewport,
			std::vector<VkViewport> const & viewports);

		void SpecifyPipelineViewportAndScissorTestState(ViewportInfo const                & viewport_infos,
			VkPipelineViewportStateCreateInfo & viewport_state_create_info);

		void SetScissorStateDynamically(
			VkCommandBuffer               command_buffer,
			uint32_t                      first_scissor,
			std::vector<VkRect2D> const & scissors);
	}
}

#endif // !VULKAN_VIEWPORT
