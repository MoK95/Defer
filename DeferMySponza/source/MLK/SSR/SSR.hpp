#pragma once

#include <tgl/tgl.h>

namespace MLK
{
	class ShaderManager;
	class GlStateManager;
	class MeshManager;
	struct LBuffer;

    struct TESTER
    {
        int a;
    };

	/// <summary>
    /// </summary>
	class SSR
	{
	public:
        SSR(ShaderManager* shaderManager, 
            GlStateManager* stateManager, 
            MeshManager* meshManager,
			GLuint width = 1280,
			GLuint height = 720);
		~SSR();

        void run(GLuint& inputTex, GLuint inputDepth, GLuint inputFBO);
        void resize(GLuint width, GLuint height);

    private:
        ShaderManager* m_shaderManager;
        GlStateManager* m_stateManager;
        MeshManager* m_meshManager;

        GLuint m_outputFbo;
        GLuint m_outputTex;
		GLuint m_outputStencil;

		GLuint m_width;
		GLuint m_height;
	};
}