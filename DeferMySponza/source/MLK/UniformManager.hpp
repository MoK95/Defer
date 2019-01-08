#pragma once

#include "Utils.hpp"
#include "ShaderStructs.hpp"

#include <unordered_map>

namespace sponza
{
	class Context;
	class Material;
}

namespace MLK
{
    struct UniformBuffer
    {
        GLuint id = 0;
        GLuint base = 0;
        GLuint usage = 0;
    };

    /// <summary>
    /// Manager that creates required uniform buffers and manages the resources in them. The manager will update resources on
    /// request. Some buffers require you to provide the information in order to update, others the uniform manager can handle.
    /// </summary>
	class UniformManager
	{
	public:
		UniformManager(const sponza::Context& scene);
		~UniformManager();

        void updateBufferData(UniformBufferId id, void* data, size_t size);

	private:
		const sponza::Context& m_scene;

		std::unordered_map<UniformBufferId, UniformBuffer> m_uniformBuffers;
		
		void createUniformBuffers();

        // Static helper functions.
        static UniformBuffer createUniformBuffer(GLuint base, size_t size, GLuint usage, void* data = nullptr);
        static void updateUniformBuffer(UniformBuffer buffer, size_t size, void* data = nullptr);
	};
}