#include "UniformManager.hpp"

// Camera not used but required for Context.hpp to compile.
#include <sponza/Camera.hpp>
#include <sponza/Context.hpp>
#include <sponza/Instance.hpp>
#include <sponza/DirectionalLight.hpp>

namespace MLK
{
    UniformBuffer UniformManager::createUniformBuffer(GLuint base, size_t size, GLuint usage, void* data)
    {
        UniformBuffer buffer;
        buffer.base = base;
        buffer.usage = usage;

        glGenBuffers(1, &buffer.id);
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.id);
        glBufferData(GL_UNIFORM_BUFFER, size, data, buffer.usage);
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer.base, buffer.id);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        return buffer;
    }

    void UniformManager::updateUniformBuffer(UniformBuffer buffer, size_t size, void* data)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.id);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

	UniformManager::UniformManager(const sponza::Context& scene) :
		m_scene(scene)
	{
		createUniformBuffers();
	}

	UniformManager::~UniformManager()
	{
	}

    void UniformManager::updateBufferData(UniformBufferId id, void* data, size_t size)
    {
        updateUniformBuffer(m_uniformBuffers.at(id), size, data);
    }

	void UniformManager::createUniformBuffers()
	{
        m_uniformBuffers[UniformBufferId::Frame] =
            createUniformBuffer(UniformBufferId::Frame, sizeof(PerFrameUniformData), GL_DYNAMIC_READ);

		m_uniformBuffers[UniformBufferId::Static] =
            createUniformBuffer(UniformBufferId::Static, sizeof(StaticUniformData), GL_STATIC_READ);

		m_uniformBuffers[UniformBufferId::Light] =
            createUniformBuffer(UniformBufferId::Light, sizeof(ShaderLight), GL_STREAM_READ);

        m_uniformBuffers[UniformBufferId::Shadow] =
            createUniformBuffer(UniformBufferId::Shadow, sizeof(ShadowUniform), GL_DYNAMIC_READ);

        m_uniformBuffers[UniformBufferId::Viewport] =
            createUniformBuffer(UniformBufferId::Viewport, sizeof(ViewportData), GL_DYNAMIC_READ);
	}
}