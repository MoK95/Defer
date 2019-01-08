#include "SMAA.hpp"

#include <assert.h>
#include "../GlStateManager.hpp"
#include "../ShaderManager.hpp"
#include "../MeshManager.hpp"
#include "../Utils.hpp"
#include "AreaTex.h"
#include "SearchTex.h"

namespace MLK
{
	SMAA::SMAA(ShaderManager* shaderManager, GlStateManager* stateManager, MeshManager* meshManager,
		GLuint width, GLuint height) :
		m_shaderManager(shaderManager), m_stateManager(stateManager), m_meshManager(meshManager),
		m_width(width), m_height(height)
	{
		// Prepare edge texture.
		glGenTextures(1, &m_edgeTex);
		glBindTexture(GL_TEXTURE_2D, m_edgeTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_BYTE, 0);

		// Prepare blend texture.
		glGenTextures(1, &m_blendTex);
		glBindTexture(GL_TEXTURE_2D, m_blendTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_BYTE, 0);

		// Shared stencil for optimisation.
		glGenRenderbuffers(1, &m_stencil);
		glBindRenderbuffer(GL_RENDERBUFFER, m_stencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, m_width, m_height);

		// Load in area texture.
		glGenTextures(1, &m_areaTex);
		glBindTexture(GL_TEXTURE_2D, m_areaTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, (GLsizei)AREATEX_WIDTH, (GLsizei)AREATEX_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, areaTexBytes);
		
		// Load in search texture.
		glGenTextures(1, &m_searchTex);
		glBindTexture(GL_TEXTURE_2D, m_searchTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (GLsizei)SEARCHTEX_WIDTH, (GLsizei)SEARCHTEX_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, searchTexBytes); 

		// Draw buffers.
		glGenFramebuffers(1, &m_edgeFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_edgeFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_edgeTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencil);

		glGenFramebuffers(1, &m_blendFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_blendFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blendTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencil);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	SMAA::~SMAA()
	{
		glDeleteTextures(1, &m_edgeTex);
		glDeleteTextures(1, &m_blendTex);
        glDeleteTextures(1, &m_searchTex);
        glDeleteTextures(1, &m_areaTex);

		glDeleteRenderbuffers(1, &m_stencil);
        
        glDeleteFramebuffers(1, &m_edgeFbo);
        glDeleteFramebuffers(1, &m_blendFbo);
	}

	void SMAA::runSMAA(GLuint input)
	{
		edgePass(input);
        weightPass();
        neighbourhoodPass(input);
	}

    void SMAA::resizeBuffers(GLuint width, GLuint height)
    {
		if (width != m_width || height != m_height)
		{
			m_width = width;
			m_height = height;

			glActiveTexture(TextureSlot::TEmpty);

			glBindTexture(GL_TEXTURE_2D, m_edgeTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_BYTE, 0);

			glBindTexture(GL_TEXTURE_2D, m_blendTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_BYTE, 0);

			glBindRenderbuffer(GL_RENDERBUFFER, m_stencil);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
    }

	void SMAA::edgePass(GLuint input)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_edgeFbo);

		m_shaderManager->useProgram(ShaderProgram::Edge);
		m_stateManager->setState(DrawPass::SMAAEdge);

        glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(TextureSlot::TInput);
		glBindTexture(GL_TEXTURE_2D, input);
        glGenerateMipmap(GL_TEXTURE_2D);

		m_meshManager->drawMeshGroup(MeshGroup::Quad);
	}

	void SMAA::weightPass()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_blendFbo);

        m_shaderManager->useProgram(ShaderProgram::Blend);
        m_stateManager->setState(DrawPass::SMAABlend);

		glClear(GL_COLOR_BUFFER_BIT);
        
		glActiveTexture(TextureSlot::TInput);
		glBindTexture(GL_TEXTURE_2D, m_edgeTex);
		glActiveTexture(TextureSlot::TArea);
		glBindTexture(GL_TEXTURE_2D, m_areaTex);
		glActiveTexture(TextureSlot::TSearch);
		glBindTexture(GL_TEXTURE_2D, m_searchTex);

		m_meshManager->drawMeshGroup(MeshGroup::Quad);
	}

	void SMAA::neighbourhoodPass(GLuint input)
	{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_shaderManager->useProgram(ShaderProgram::Resolve);
        m_stateManager->setState(DrawPass::SMAAResolve);

		glActiveTexture(TextureSlot::TInput);
		glBindTexture(GL_TEXTURE_2D, input);
		glActiveTexture(TextureSlot::TSearch);
		glBindTexture(GL_TEXTURE_2D, m_blendTex);

		m_meshManager->drawMeshGroup(MeshGroup::Quad);

        glActiveTexture(TextureSlot::TInput);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(TextureSlot::TSearch);
        glBindTexture(GL_TEXTURE_2D, 0);
	}
}