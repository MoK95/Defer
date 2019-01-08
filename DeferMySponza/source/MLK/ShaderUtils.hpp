#pragma once

#include "Utils.hpp"

namespace MLK
{
    namespace ShaderUtils
    {        
        // Creates a shader from the given string. If a shader prefix is provided it is prepended to the shader source.
        GLuint createShader(GLuint shaderType, const std::string& source, const std::string& shaderPrefix = "");

        // Creates a new program given all required parameters. Should potentially be split into multiple functions
        // however this ensures nothing is missed when adding programs.
        GLuint createProgram(const std::vector<GLuint>& shaderIds,
            const std::vector<AttribLocation>& attribLocations,
            const std::vector<FragDataLocation>& fragLocations,
            const std::vector<UniformBufferId>& uboIds,
            const std::vector<TextureSlot>& textureIds);

        // Links a given program asserting if failure.
        void linkProgram(GLuint programId);

        // Binds the given attributes to the given program. See g_vertexLocationToName map for variable names.
        void bindAttributes(GLuint programId, const std::vector<AttribLocation>& attributes);

        // Binds the given frag data locations to the given program. See g_fragLocationToName map for variable names.
        void bindFragDataLocations(GLuint programId, const std::vector<FragDataLocation>& fragOutputs);
    }
}