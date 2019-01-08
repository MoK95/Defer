#pragma once

#include <tgl/tgl.h>

namespace MLK
{
	class ShaderManager;
	class GlStateManager;
	class MeshManager;
	struct LBuffer;

	/// <summary>
    /// Implementation to help manage witching of OpenGl variables. The different passes require multiple
    /// GL variables to be set, this groups them and ensures they aren't switched unnecessarily.
    /// </summary>
	class SMAA
	{
	public:
		SMAA(ShaderManager* shaderManager, 
			GlStateManager* stateManager, 
			MeshManager* meshManager,
			GLuint width = 1280,
			GLuint height = 720);
		~SMAA();

        // SMAA should be the final step so output to screen by default.
		void runSMAA(GLuint input);
        void resizeBuffers(GLuint width, GLuint height);

	private:
		void edgePass(GLuint input);
		void weightPass();
		void neighbourhoodPass(GLuint input);

	private:
		ShaderManager* m_shaderManager;
		GlStateManager* m_stateManager;
		MeshManager* m_meshManager;
		GLuint m_width = 1280;
		GLuint m_height = 720;

		GLuint m_edgeFbo;
		GLuint m_blendFbo;
		GLuint m_stencil;
		GLuint m_edgeTex;
		GLuint m_blendTex;
		GLuint m_areaTex;
		GLuint m_searchTex;
	};
}