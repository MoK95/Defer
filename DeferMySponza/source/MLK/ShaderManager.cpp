#include "ShaderManager.hpp"

#include "ShaderUtils.hpp"

#include <tygra/FileHelper.hpp>

namespace MLK
{
    namespace SU = ShaderUtils;

    std::string ShaderManager::s_shaderStructures = "";
    std::string ShaderManager::s_smaaFunctions = "";

    ShaderManager::ShaderManager()
    {
        if (s_shaderStructures.empty())
        {
            s_shaderStructures = tygra::createStringFromFile("resource:///ShaderStructures.glsl");
        }
        if (s_smaaFunctions.empty())
        {
            s_smaaFunctions = tygra::createStringFromFile("resource:///SMAA.glsl");
        }

        m_currentProgram = ShaderProgram::NoProgram;

        createPrograms();
    }

    ShaderManager::~ShaderManager()
    {
        deletePrograms();
    }

    void ShaderManager::recompileShaders()
    {
        // Ensure old programs are deleted.
        deletePrograms();

        // Re create programs.
        createPrograms();
    }

    void ShaderManager::createShaders()
    {
        // Vertex Shaders.
        m_shaders[ShaderId::GBufferVS] = SU::createShader(GL_VERTEX_SHADER, tygra::createStringFromFile("resource:///GBufferVS.glsl"), s_shaderStructures);
        m_shaders[ShaderId::QuadVS] = SU::createShader(GL_VERTEX_SHADER, tygra::createStringFromFile("resource:///QuadVS.glsl"));
        m_shaders[ShaderId::LightVolumeVS] = SU::createShader(GL_VERTEX_SHADER, tygra::createStringFromFile("resource:///LightVolumeVS.glsl"), s_shaderStructures);
        m_shaders[ShaderId::ShadowsVS] = SU::createShader(GL_VERTEX_SHADER, tygra::createStringFromFile("resource:///ShadowVS.glsl"), s_shaderStructures);

        // SMAA
        m_shaders[ShaderId::EdgeVS] = SU::createShader(GL_VERTEX_SHADER, tygra::createStringFromFile("resource:///EdgeVS.glsl"), s_smaaFunctions);
        m_shaders[ShaderId::BlendVS] = SU::createShader(GL_VERTEX_SHADER, tygra::createStringFromFile("resource:///BlendVS.glsl"), s_smaaFunctions);
        m_shaders[ShaderId::ResolveVS] = SU::createShader(GL_VERTEX_SHADER, tygra::createStringFromFile("resource:///ResolveVS.glsl"), s_smaaFunctions);

        // Fragment Shaders.
        m_shaders[ShaderId::AmbientFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///AmbientFS.glsl"), s_shaderStructures);
        m_shaders[ShaderId::PointFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///PointLightFS.glsl"), s_shaderStructures);
        m_shaders[ShaderId::SpotFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///SpotLightFS.glsl"), s_shaderStructures);
        m_shaders[ShaderId::SpotShadowFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///SpotShadowFS.glsl"), s_shaderStructures);
        m_shaders[ShaderId::GBufferFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///GBufferFS.glsl"), s_shaderStructures);
        m_shaders[ShaderId::ShadowsFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///ShadowFS.glsl"));

        // SSR
        m_shaders[ShaderId::SSRFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///SSRFS.glsl"), s_shaderStructures);
        
        // SMAA
        m_shaders[ShaderId::EdgeFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///EdgeFS.glsl"), s_smaaFunctions);
        m_shaders[ShaderId::BlendFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///BlendFS.glsl"), s_smaaFunctions);
        m_shaders[ShaderId::ResolveFS] = SU::createShader(GL_FRAGMENT_SHADER, tygra::createStringFromFile("resource:///ResolveFS.glsl"), s_smaaFunctions);
    }

    void ShaderManager::deleteShaders()
    {
        for (const auto shader : m_shaders)
        {
            glDeleteShader(shader.second);
        }
        m_shaders.clear();
    }

    void ShaderManager::createPrograms()
    {
        createShaders();

        m_programs[ShaderProgram::GBufferProgram] = SU::createProgram(
        { m_shaders.at(ShaderId::GBufferVS), m_shaders.at(ShaderId::GBufferFS) },
        { AttribLocation::Position, AttribLocation::Normal, AttribLocation::UV0, AttribLocation::InstanceID },
        { FragDataLocation::GBufferPosition, FragDataLocation::GBufferNormal, FragDataLocation::GBufferMaterial },
        { UniformBufferId::Frame },
        { }
        );

        m_programs[ShaderProgram::Ambient] = SU::createProgram(
        { m_shaders.at(ShaderId::QuadVS), m_shaders.at(ShaderId::AmbientFS) },
        { AttribLocation::Position, AttribLocation::UV0 },
        { FragDataLocation::OutColour },
        { UniformBufferId::Frame, UniformBufferId::Static },
        { TextureSlot::TPosition, TextureSlot::TNormal, TextureSlot::TMaterial }
        );

        m_programs[ShaderProgram::SpotLight] = SU::createProgram(
        { m_shaders.at(ShaderId::LightVolumeVS), m_shaders.at(ShaderId::SpotFS) },
        { AttribLocation::Position },
        { FragDataLocation::OutColour },
        { UniformBufferId::Frame, UniformBufferId::Static, UniformBufferId::Light },
        { TextureSlot::TPosition, TextureSlot::TNormal, TextureSlot::TMaterial }
        );

        m_programs[ShaderProgram::SpotShadow] = SU::createProgram(
        { m_shaders.at(ShaderId::LightVolumeVS), m_shaders.at(ShaderId::SpotShadowFS) },
        { AttribLocation::Position },
        { FragDataLocation::OutColour },
        { UniformBufferId::Frame, UniformBufferId::Static, UniformBufferId::Light, UniformBufferId::Shadow },
        { TextureSlot::TPosition, TextureSlot::TNormal, TextureSlot::TMaterial, TextureSlot::TShadow }
        );

        m_programs[ShaderProgram::PointLight] = SU::createProgram(
        { m_shaders.at(ShaderId::LightVolumeVS), m_shaders.at(ShaderId::PointFS) },
        { AttribLocation::Position },
        { FragDataLocation::OutColour },
        { UniformBufferId::Frame, UniformBufferId::Static, UniformBufferId::Light },
        { TextureSlot::TPosition, TextureSlot::TNormal, TextureSlot::TMaterial }
        );

        m_programs[ShaderProgram::Shadows] = SU::createProgram(
        { m_shaders.at(ShaderId::ShadowsVS), m_shaders.at(ShaderId::ShadowsFS) },
        { AttribLocation::Position, AttribLocation::Normal, AttribLocation::UV0, AttribLocation::InstanceID },
        { },
        { UniformBufferId::Frame, UniformBufferId::Shadow },
        { }
        );

        m_programs[ShaderProgram::SSRProgram] = SU::createProgram(
        { m_shaders.at(ShaderId::QuadVS), m_shaders.at(ShaderId::SSRFS) },
        { AttribLocation::Position, AttribLocation::UV0 },
        { FragDataLocation::OutColour },
        { UniformBufferId::Frame, UniformBufferId::Viewport },
        { TextureSlot::TPosition, TextureSlot::TNormal, TextureSlot::TMaterial, TextureSlot::TInput, TextureSlot::TSearch}
        );

        m_programs[ShaderProgram::Edge] = SU::createProgram(
        { m_shaders.at(ShaderId::EdgeVS), m_shaders.at(ShaderId::EdgeFS) },
        { AttribLocation::Position, AttribLocation::UV0 },
        { FragDataLocation::OutColour },
        { UniformBufferId::Viewport },
        { TextureSlot::TInput }
        );

        m_programs[ShaderProgram::Blend] = SU::createProgram(
        { m_shaders.at(ShaderId::BlendVS), m_shaders.at(ShaderId::BlendFS) },
        { AttribLocation::Position, AttribLocation::UV0 },
        { FragDataLocation::OutColour },
        { UniformBufferId::Viewport },
        { TextureSlot::TInput, TextureSlot::TArea, TextureSlot::TSearch }
        );

        m_programs[ShaderProgram::Resolve] = SU::createProgram(
        { m_shaders.at(ShaderId::ResolveVS), m_shaders.at(ShaderId::ResolveFS) },
        { AttribLocation::Position, AttribLocation::UV0 },
        { FragDataLocation::OutColour },
        { UniformBufferId::Viewport },
        { TextureSlot::TInput, TextureSlot::TSearch }
        );

        deleteShaders();
    }

    void ShaderManager::deletePrograms()
    {
        for (auto program : m_programs)
        {
            glDeleteProgram(program.second);
        }
        m_programs.clear();
    }

    GLuint ShaderManager::getProgramId(ShaderProgram program) const
    {
        return m_programs.at(program);
    }

    void ShaderManager::useProgram(ShaderProgram program)
    {
        if (m_currentProgram == program || program == ShaderProgram::NoProgram)
        {
            return;
        }

        m_currentProgram = program;
        glUseProgram(m_programs.at(program));
    }
}