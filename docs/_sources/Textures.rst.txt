Textures
############

	.. doxygenfunction:: LavaCake::Framework::createTextureBuffer(int width, int height, int depth, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM)
		:project: LavaCake
	.. doxygenfunction:: LavaCake::Framework::createTextureBuffer(Queue* queue, CommandBuffer& cmdBuff, std::string filename, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
		:project: LavaCake
	.. doxygenfunction:: LavaCake::Framework::createTextureBuffer(Queue* queue, CommandBuffer& cmdBuff, std::vector<unsigned char>* data, int width, int height, int depth, int nbChannel, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		:project: LavaCake
	.. doxygenfunction:: LavaCake::Framework::createCubeMap
		:project: LavaCake
	.. doxygenfunction:: LavaCake::Framework::createAttachment
		:project: LavaCake
	.. doxygenfunction:: LavaCake::Framework::createStorageImage
		:project: LavaCake

	.. doxygenenum:: LavaCake::Framework::attachmentType
		:project: LavaCake