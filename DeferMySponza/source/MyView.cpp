#include "MyView.hpp"

#include "MLK/ShaderManager.hpp"
#include "MLK/MeshManager.hpp"
#include "MLK/UniformManager.hpp"
#include "MLK/GlStateManager.hpp"
#include "MLK/MaterialManager.hpp"
#include "MLK/SMAA/SMAA.hpp"
#include "MLK/SSR/SSR.hpp"

#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sponza/sponza.hpp>
#include <iostream>
#include <cassert>

namespace MU = M::Utils;

MyView::MyView()
{
}

MyView::~MyView() 
{
}

void MyView::setScene(const sponza::Context * sponza)
{
    scene_ = sponza;
}

void MyView::recompileShaders()
{
    m_shaderManager->recompileShaders();
}

void MyView::windowViewWillStart(tygra::Window * window)
{
	assert(scene_ != nullptr);

    updateAspectRatio(false);

    // Create required resources.
    m_gBuffer = MU::createGBuffer(m_windowWidth, m_windowHeight);
    m_lBuffer = MU::createLBuffer(m_windowWidth, m_windowHeight, m_gBuffer.depth);
    m_shadowMap = MU::createShadowMap(m_shadowRes);

    // Create managers.
    m_meshManager = new M::MeshManager(*scene_);

    m_materialManager = new M::MaterialManager(*scene_);

    m_uniformManager = new M::UniformManager(*scene_);

    m_shaderManager = new M::ShaderManager();

    m_glStateManager = new M::GlStateManager();
    
    m_ssr = new MLK::SSR(m_shaderManager, m_glStateManager, m_meshManager, m_windowWidth, m_windowHeight);

    m_smaa = new M::SMAA(m_shaderManager, m_glStateManager, m_meshManager, m_windowWidth, m_windowHeight);

    // Set static data on start.
    updateStaticData();
    updateViewportData();
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);

	updateAspectRatio();
}

void MyView::windowViewDidStop(tygra::Window * window)
{
    delete m_meshManager;
    delete m_materialManager;
    delete m_uniformManager;
    delete m_shaderManager;
    delete m_glStateManager;
    delete m_ssr;
    delete m_smaa;
}

void MyView::updateStaticData()
{
    m_staticData.GlobalLights.AmbientIntensity = (const glm::vec3&)scene_->getAmbientLightIntensity();
    const auto materialData = m_materialManager->getMaterialData();
    memcpy(m_staticData.Materials, materialData.data(), 16 * sizeof(materialData[0]));

    const auto& directionalLights = scene_->getAllDirectionalLights();
    for (int i = 0; i < directionalLights.size(); ++i)
    {
        m_staticData.GlobalLights.DirectionalLights[i] = M::DirectionalLight(directionalLights[i]);
    }

    m_staticData.GlobalLights.DirectionalLightCount = (GLuint)directionalLights.size();

    m_uniformManager->updateBufferData(M::UniformBufferId::Static, &m_staticData, sizeof(m_staticData));
}

void MyView::updateFrameData()
{
    m_frameData.EyePosition = (const glm::vec3&)scene_->getCamera().getPosition();
    m_frameData.ViewProjectionMatrix = MLK::Utils::getViewProjectionMatrix(*scene_, m_aspectRatio);

    unsigned int counter = 0;
    for (const auto& instance : scene_->getAllInstances())
    {
        const auto& modelTransform = glm::mat4(
            (const glm::mat4x3&)instance.getTransformationMatrix());

		// Special case required for only sponza's floor to be reflective.
		GLuint materialId = 0;
		if (instance.getMeshId() == 311)
		{
			materialId = m_materialManager->lookupMaterialId(100);
		}
		else
		{
			materialId = m_materialManager->lookupMaterialId(instance.getMaterialId());
		}

        const M::MeshInstanceData instanceData(modelTransform, materialId);

        m_frameData.InstanceData[counter] = instanceData;
        ++counter;
    }

    m_uniformManager->updateBufferData(M::UniformBufferId::Frame, &m_frameData, sizeof(m_frameData));
}

void MyView::updateShadowData()
{
    auto shadowPerspective = glm::perspective(m_lightData.Angle * 2, 1.f, 0.1f, m_lightData.Range);
    auto shadowView = glm::lookAt(m_lightData.Position, m_lightData.Position + m_lightData.Direction, (const glm::vec3&)scene_->getUpDirection());
    m_shadowData.VP = shadowPerspective * shadowView;
    m_uniformManager->updateBufferData(M::UniformBufferId::Shadow, &m_shadowData, sizeof(m_shadowData));
}

void MyView::updateViewportData()
{
    M::ViewportData data;
    data.PixelHeight = 1.f / m_windowHeight;
    data.PixelWidth = 1.f / m_windowWidth;
    data.ScreenHeight = m_windowHeight;
    data.ScreenWidth = m_windowWidth;
    m_uniformManager->updateBufferData(M::UniformBufferId::Viewport, &data, sizeof(data));

    // Resize necessary frambuffers.
    MU::resizeFramebuffers(m_windowWidth, m_windowHeight, m_gBuffer, m_lBuffer);
    m_ssr->resize(m_windowWidth, m_windowHeight);
    m_smaa->resizeBuffers(m_windowWidth, m_windowHeight);
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(scene_ != nullptr);

    // Update per frame uniforms.
    updateFrameData();
    
    drawGBuffer();
    
    drawAmbient();

    drawPointLights();

    drawSpotLights();

    GLuint inputTex = m_lBuffer.color;

    if (m_enableSSR)
    {
        m_ssr->run(inputTex, m_lBuffer.depth, m_lBuffer.fbo);
    }

    if (m_useSMAA)
    {
        m_smaa->runSMAA(inputTex);
    }

    else
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, m_windowWidth, m_windowHeight, 0, 0, m_windowWidth, m_windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
}

void MyView::toggleShadows()
{
    m_enableShadows = !m_enableShadows;
}

void MyView::toggleSSR()
{
    m_enableSSR = !m_enableSSR;
}

void MyView::toggleSMAA()
{
    m_useSMAA = !m_useSMAA;
}

void MyView::drawGBuffer()
{
    MU::unbindGBufferTextures();
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);
    m_glStateManager->setState(M::DrawPass::GBufferPass); // Set GL variables.
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    m_shaderManager->useProgram(M::ShaderProgram::GBufferProgram); // Use program.
    m_meshManager->drawMeshGroup(M::MeshGroup::Sponza); // Draw Sponza.
}

void MyView::drawAmbient()
{
    MU::bindGBufferTextures(m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_lBuffer.fbo);
    m_glStateManager->setState(M::DrawPass::AmbientPass);
    glClear(GL_COLOR_BUFFER_BIT);
    m_shaderManager->useProgram(M::ShaderProgram::Ambient);
    m_meshManager->drawMeshGroup(M::MeshGroup::Quad);
}

void MyView::drawPointLights()
{
    m_shaderManager->useProgram(M::ShaderProgram::PointLight); // Use program.

    for (const auto& point : scene_->getAllPointLights())
    {
        m_lightData = M::ShaderLight(point);
        m_uniformManager->updateBufferData(M::UniformBufferId::Light, &m_lightData, sizeof(m_lightData));

        m_glStateManager->setState(M::DrawPass::LightStencilPass);
        m_meshManager->drawMeshGroup(M::MeshGroup::Sphere);

        m_glStateManager->setState(M::DrawPass::LightShadingPass);
        m_meshManager->drawMeshGroup(M::MeshGroup::Sphere);
    }
}

void MyView::drawSpotLights()
{
    for (const auto& spot : scene_->getAllSpotLights())
    {
        m_lightData = M::ShaderLight(spot);
        m_uniformManager->updateBufferData(M::UniformBufferId::Light, &m_lightData, sizeof(m_lightData));

        if (m_enableShadows && m_lightData.CastsShadows)
        {
            updateShadowData();

            m_shaderManager->useProgram(M::ShaderProgram::Shadows);

            glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMap.fbo);
            m_glStateManager->setState(M::DrawPass::ShadowMapPass);

            glActiveTexture(M::TextureSlot::TShadow);
            glBindTexture(GL_TEXTURE_2D, 0);

            glClear(GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, m_shadowRes, m_shadowRes);

            m_meshManager->drawMeshGroup(M::MeshGroup::Sponza);

            glViewport(0, 0, m_windowWidth, m_windowHeight);

            glBindTexture(GL_TEXTURE_2D, m_shadowMap.depthTex);
            glActiveTexture(M::TextureSlot::TEmpty);
            glBindTexture(GL_TEXTURE_2D, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, m_lBuffer.fbo);

            m_shaderManager->useProgram(M::ShaderProgram::SpotShadow); // Use program.

            m_glStateManager->setState(M::DrawPass::LightStencilPass);
            m_meshManager->drawMeshGroup(M::MeshGroup::Cone);

            m_glStateManager->setState(M::DrawPass::LightShadingPass);
            m_meshManager->drawMeshGroup(M::MeshGroup::Cone);
        }
        else
        {
            m_shaderManager->useProgram(M::ShaderProgram::SpotLight); // Use program.

            m_glStateManager->setState(M::DrawPass::LightStencilPass);
            m_meshManager->drawMeshGroup(M::MeshGroup::Cone);

            m_glStateManager->setState(M::DrawPass::LightShadingPass);
            m_meshManager->drawMeshGroup(M::MeshGroup::Cone);
        }
    }
}

void MyView::updateAspectRatio(bool resizeFramebuffers)
{
    GLint viewportSize[4];
    glGetIntegerv(GL_VIEWPORT, viewportSize);

    m_windowWidth = viewportSize[2];
    m_windowHeight = viewportSize[3];

    m_aspectRatio = (float)viewportSize[2] / (float)viewportSize[3];

    if (resizeFramebuffers)
    {
        updateViewportData();
    }
}