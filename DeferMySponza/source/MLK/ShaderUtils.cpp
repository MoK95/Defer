#include "ShaderUtils.hpp"

#include <iostream>

namespace MLK
{
    namespace ShaderUtils
    {
        std::unordered_map<AttribLocation, std::string> g_vertexLocationToName =
        {
            { AttribLocation::Position, "Position" },
            { AttribLocation::Normal, "Normal" },
            { AttribLocation::UV0, "UV0" },
            { AttribLocation::InstanceID, "InstanceID" },
        };

        std::unordered_map<FragDataLocation, std::string> g_fragLocationToName =
        {
            { FragDataLocation::OutColour, "OutColour" },
            { FragDataLocation::GBufferPosition, "GBufferPosition"},
            { FragDataLocation::GBufferNormal, "GBufferNormal" },
            { FragDataLocation::GBufferColour, "GBufferColour" },
            { FragDataLocation::GBufferMaterial, "GBufferMaterial" }
        };

        std::unordered_map<UniformBufferId, std::string> g_uniformToName = 
        {
            { UniformBufferId::Frame, "PerFrameData"},
            { UniformBufferId::Light, "LightData" },
            { UniformBufferId::Static, "StaticData"},
            { UniformBufferId::Shadow, "ShadowData"},
            { UniformBufferId::Viewport, "ViewportData" }
        };

        std::unordered_map<TextureSlot, std::string> g_textureToName =
        {
            { TextureSlot::TPosition, "Positions" },
            { TextureSlot::TNormal, "Normals" },
            { TextureSlot::TMaterial, "MaterialIDs" },
            { TextureSlot::TShadow, "ShadowMap" },
            { TextureSlot::TInput, "Input" },
            { TextureSlot::TArea, "Area" },
            { TextureSlot::TSearch, "Search" }
        };

        GLuint createProgram(const std::vector<GLuint>& shaderIds,
            const std::vector<AttribLocation>& attribLocations,
            const std::vector<FragDataLocation>& fragLocations,
            const std::vector<UniformBufferId>& uboIds, 
            const std::vector<TextureSlot>& textureIds)
        {
            GLuint programId = glCreateProgram();

            for (const auto shader : shaderIds)
            {
                glAttachShader(programId, shader);
            }

            for (auto attribute : attribLocations)
            {
                glBindAttribLocation(programId, attribute, g_vertexLocationToName.at(attribute).c_str());
            }

            for (int i = 0; i < fragLocations.size(); ++i)
            {
                glBindFragDataLocation(programId, i, g_fragLocationToName.at(fragLocations[i]).c_str());
            }

            linkProgram(programId);

            for (const auto ubo : uboIds)
            {
                auto uniformLocation = glGetUniformBlockIndex(programId, g_uniformToName.at(ubo).c_str());
                glUniformBlockBinding(programId, uniformLocation, ubo);
            }

            // Program must be in use to bind textures.
            glUseProgram(programId);
            for (const auto texture : textureIds)
            {
                auto location = glGetUniformLocation(programId, g_textureToName.at(texture).c_str());
                glUniform1i(location, Utils::getTextureID(texture));
            }
            glUseProgram(0);

            return programId;
        }

        GLuint createShader(GLuint shaderType, const std::string& source, const std::string& shaderPrefix)
        {
            // Assert to catch incorrect shader file name.
            assert(!source.empty());

            GLuint id = glCreateShader(shaderType);

            std::string completeSource = shaderPrefix + source;
            auto shaderString = (const GLchar*)completeSource.c_str();

            glShaderSource(id, 1, &shaderString, 0);

            glCompileShader(id);

            int success = 0;
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);

            if (success == GL_FALSE)
            {
                int logLength = 0;
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

                auto log = std::vector<GLchar>(logLength);

                glGetShaderInfoLog(id, logLength, nullptr, log.data());

                std::cerr << log.data() << std::endl;

                glDeleteShader(id);

                // Assert to catch shader compilation error.
                assert(false);
            }

            return id;
        }

        void linkProgram(GLuint programId)
        {
            glLinkProgram(programId);

            GLint success = 0;
            glGetProgramiv(programId, GL_LINK_STATUS, &success);

            if (success == GL_FALSE)
            {
                int logLength = 0;
                glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);

                if (logLength > 0)
                {
                    auto log = std::vector<GLchar>(logLength);

                    glGetProgramInfoLog(programId, logLength, NULL, log.data());
                    std::cerr << log.data() << std::endl;
                }

                // Assert to catch program link error.
                assert(false);
            }
        }

        void bindAttributes(GLuint programId, const std::vector<AttribLocation>& attributes)
        {
            for (auto attribute : attributes)
            {
                glBindAttribLocation(programId, attribute, g_vertexLocationToName.at(attribute).c_str());
            }
        }

        void bindFragDataLocations(GLuint programId, const std::vector<FragDataLocation>& fragDataLocations)
        {
			for (int i = 0; i < fragDataLocations.size(); ++i)
			{
				glBindFragDataLocation(programId, i, g_fragLocationToName.at(fragDataLocations[i]).c_str());
			}
        }
    }
}