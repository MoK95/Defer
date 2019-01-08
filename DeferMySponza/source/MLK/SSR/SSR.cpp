#include "SSR.hpp"

#include <assert.h>

#include "../GlStateManager.hpp"
#include "../ShaderManager.hpp"
#include "../MeshManager.hpp"
#include "../Utils.hpp"

namespace MLK
{
    SSR::SSR(ShaderManager* shaderManager,
        GlStateManager* stateManager,
        MeshManager* meshManager,
		GLuint width,
		GLuint height) : 
        m_shaderManager(shaderManager),
        m_stateManager(stateManager),
        m_meshManager(meshManager),
		m_width(width),
		m_height(height)
    {
        glGenFramebuffers(1, &m_outputFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_outputFbo);

        glGenTextures(1, &m_outputTex);
        glBindTexture(GL_TEXTURE_2D, m_outputTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_BYTE, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outputTex, 0);

		auto success = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		assert(success == GL_FRAMEBUFFER_COMPLETE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
	}

	SSR::~SSR()
	{
        glDeleteFramebuffers(1, &m_outputFbo);
        glDeleteTextures(1, &m_outputTex);
		glDeleteRenderbuffers(1, &m_outputStencil);
	}

    void SSR::run(GLuint& inputTex, GLuint inputDepth, GLuint inputFBO)
    {
		glActiveTexture(TextureSlot::TInput);
		glBindTexture(GL_TEXTURE_2D, inputTex);
		glActiveTexture(TextureSlot::TSearch);
		glBindTexture(GL_TEXTURE_2D, inputDepth);

		m_shaderManager->useProgram(ShaderProgram::SSRProgram);
		m_stateManager->setState(DrawPass::SSRPass);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, inputFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_outputFbo);
		glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_outputFbo);

        m_meshManager->drawMeshGroup(MeshGroup::Quad);

		inputTex = m_outputTex;
    }

    void SSR::resize(GLuint width, GLuint height)
    {
		if (width != m_width || height != m_height)
		{
			m_width = width;
			m_height = height;

			glActiveTexture(TextureSlot::TEmpty);

			glBindTexture(GL_TEXTURE_2D, m_outputTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_BYTE, 0);

			glBindTexture(GL_TEXTURE_2D, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, m_outputStencil);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, m_width, m_height);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
    }
}