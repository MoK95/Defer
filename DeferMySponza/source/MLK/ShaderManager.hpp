#pragma once

#include <glm/glm.hpp>
#include <tgl/tgl.h>
#include <unordered_map>

namespace MLK
{
    /// <summary>
    /// Enum to store and interact with specific programs more explicitly.
    /// </summary>
    enum ShaderProgram
    {
		NoProgram,
		GBufferProgram,
        Ambient,
        SpotLight,
        SpotShadow,
        PointLight,
		Shadows,
        SSRProgram,
        Edge,
        Blend,
        Resolve
    };

    /// <summary>
    /// Class implementation to handle the compilation of shaders files and linking of shader programs.
    /// </summary>
	class ShaderManager
	{
    public:
        ShaderManager();
        ~ShaderManager();

        void useProgram(ShaderProgram program);
        GLuint getProgramId(ShaderProgram program) const;

        void recompileShaders();

    private:
        enum ShaderId
        {
			AmbientFS,
            SpotFS,
            SpotShadowFS,
            PointFS,
            GBufferVS,
            GBufferFS,
			QuadVS,
            LightVolumeVS,
			ShadowsVS,
			ShadowsFS,
            SSRFS,
            EdgeVS,
            EdgeFS,
            BlendVS,
            BlendFS,
            ResolveVS,
            ResolveFS
        };

        void createShaders();
        void deleteShaders();

        void createPrograms();
        void deletePrograms();

        std::unordered_map<ShaderId, GLuint> m_shaders;
        std::unordered_map<ShaderProgram, GLuint> m_programs;

        ShaderProgram m_currentProgram;
        
        static std::string s_shaderStructures;
        static std::string s_smaaFunctions;
	};
}