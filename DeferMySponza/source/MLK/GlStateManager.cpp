#include "GlStateManager.hpp"

#include <assert.h>
#include <tgl/tgl.h>

namespace MLK
{
    GlStateManager::GlStateManager()
    {
		// Blend settings.
        glBlendColor(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

		// Clear values.
        glClearColor(0.f, 0.f, 0.25f, 0.f);
		glClearStencil(0x80);
		glClearDepth(1.0);

		// Possible to utilise strings and have a user define the state changes, as I am the only user keeping
		// these states internal is easier to keep track of.
        m_passFunctions[DrawPass::GBufferPass] = [this]() { setGBufferPass(); };
        m_passFunctions[DrawPass::AmbientPass] = [this]() { setAmbientPass(); };
        m_passFunctions[DrawPass::FullScreenPass] = [this]() { setFullScreenPass(); };
        m_passFunctions[DrawPass::LightStencilPass] = [this]() { setLightStencilPass(); };
        m_passFunctions[DrawPass::LightShadingPass] = [this]() { setLightShadingPass(); };
		m_passFunctions[DrawPass::ShadowMapPass] = [this]() { setShadowMapPass(); };
        m_passFunctions[DrawPass::SMAAEdge] = [this]() { setSMAAEdge(); };
        m_passFunctions[DrawPass::SMAABlend] = [this]() { setSMAABlend(); };
        m_passFunctions[DrawPass::SMAAResolve] = [this]() { setSMAAResolve(); };
        m_passFunctions[DrawPass::SSRPass] = [this]() { setSSRPass(); };
    }

    GlStateManager::~GlStateManager()
    {

    }

    void GlStateManager::setState(DrawPass pass)
    {
        if (m_currentPass == pass)
        {
            return;
        }

        m_currentPass = pass;
        m_passFunctions.at(pass)();
    }

    void GlStateManager::setGBufferPass()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

		// Do depth test and write values.
		glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);

		// Always write 0 in stencil test.
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xff);
		glStencilFunc(GL_ALWAYS, 0, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        // Don't blend.
        glDisable(GL_BLEND);
    }

    void GlStateManager::setAmbientPass()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);
        glStencilFunc(GL_EQUAL, 0, 0xff);

        glDisable(GL_BLEND);
    }

    void GlStateManager::setFullScreenPass()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);
        glStencilFunc(GL_EQUAL, 0, 0xff);

        glEnable(GL_BLEND); // Do blend.
        glBlendColor(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
    }

    void GlStateManager::setLightStencilPass()
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        glDisable(GL_CULL_FACE);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_NOTEQUAL, 0x80, 0xff);
        glStencilMask(0xff);
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

        glDisable(GL_BLEND);
    }

    void GlStateManager::setLightShadingPass()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glDisable(GL_DEPTH_TEST);

		glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 1, 0xff);
        glStencilMask(0xff);
        glStencilOp(GL_KEEP, GL_DECR, GL_DECR);

		glEnable(GL_BLEND);
		glBlendColor(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
    }

	void GlStateManager::setShadowMapPass()
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);

		glDisable(GL_STENCIL_TEST);

		glDisable(GL_BLEND);
	}

    void GlStateManager::setSMAAEdge()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_FALSE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0x00, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        glStencilMask(0xff);

        glDisable(GL_BLEND);
    }

    void GlStateManager::setSMAABlend()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 0x01, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
        glStencilMask(0xff);

        glDisable(GL_BLEND);
    }

    void GlStateManager::setSMAAResolve()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDisable(GL_DEPTH_TEST);

        glDisable(GL_STENCIL_TEST);

        glDisable(GL_BLEND);
    }

    void GlStateManager::setSSRPass()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDisable(GL_DEPTH_TEST);

        glDisable(GL_STENCIL_TEST);

        glEnable(GL_BLEND);
		glBlendColor(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
    }
}