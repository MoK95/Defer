#pragma once

#include "MLK/Utils.hpp"
#include "MLK/ShaderStructs.hpp"

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>
#include <memory>

namespace MLK
{
    enum MeshGroup;
    enum ShaderProgram;
    enum DrawPass;

    class MeshManager;
    class MaterialManager;
    class ShaderManager;
    class UniformManager;
    class GlStateManager;
    class SMAA;
    class SSR;
}

namespace M = MLK;

class MyView : public tygra::WindowViewDelegate
{
public:
    MyView();

    ~MyView();

    void setScene(const sponza::Context * sponza);

private:
    void windowViewWillStart(tygra::Window * window) override;

    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;

    void windowViewRender(tygra::Window * window) override;

    const sponza::Context * scene_;

public:
    void recompileShaders();

    void toggleShadows();
    void toggleSSR();
    void toggleSMAA();

private:
    void updateStaticData();
    void updateFrameData();
    void updateShadowData();
    void updateViewportData();

    // Internal drawing calls which allows for quick addition/removal of certain steps.
    // These could be made public to allow for the aspects that are drawn to be chosen.
    void drawGBuffer();
    void drawAmbient();
    void drawPointLights();
    void drawSpotLights();

    // Updates the aspect ratio required for calculate view/projection matrix.
    void updateAspectRatio(bool resizeFramebuffers = true);

private:
    GLint m_windowWidth = 0;
    GLint m_windowHeight = 0;
    GLuint m_shadowRes = 2048;
    float m_aspectRatio = 0;

    MLK::PerFrameUniformData m_frameData;
    MLK::StaticUniformData m_staticData;
    MLK::ShaderLight m_lightData;
    MLK::ShadowUniform m_shadowData;

    M::MeshManager* m_meshManager = nullptr;
    M::MaterialManager* m_materialManager = nullptr;
    M::ShaderManager* m_shaderManager = nullptr;
    M::UniformManager* m_uniformManager = nullptr;
    M::GlStateManager* m_glStateManager = nullptr;
    M::SSR* m_ssr = nullptr;
    M::SMAA* m_smaa = nullptr;

    bool m_enableShadows = true;
    bool m_enableSSR = true;
    bool m_useSMAA = true;

private:
    M::GBuffer m_gBuffer;
    M::LBuffer m_lBuffer;
    M::ShadowMap m_shadowMap;

};
