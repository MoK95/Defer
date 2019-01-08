#pragma once

#include <sponza/sponza_fwd.hpp>
#define TGL_TARGET_GL_4_3
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>
#include <memory>

namespace MLK
{
    /// <summary>
    /// Stores shadow map FBO and texture.
    /// </summary>
    struct ShadowMap
    {
        GLuint fbo = 0;
        GLuint depthTex = 0;
    };

    /// <summary>
    /// Stores all LBuffer related GL resources.
    /// </summary>
	struct LBuffer
	{
		GLuint fbo = 0;
		GLuint color = 0;
		GLuint depth = 0;
        GLuint stencil = 0;
	};

    /// <summary>
    /// Stores all GBuffer related GL resources.
    /// </summary>
    struct GBuffer
    {
        GLuint fbo = 0;
		GLuint depth = 0;
        GLuint stencil = 0;
        GLuint posTex = 0;
        GLuint normTex = 0;
        GLuint matTex = 0;
    };

    /// <summary>
    /// Enum to store attribute locations.
    /// </summary>
    enum AttribLocation
    {
        Position = 0,
        Normal = 1,
        UV0 = 2,
        InstanceID = 3
    };

    /// <summary>
    /// Enum to store fragment data output locations.
    /// </summary>
    enum FragDataLocation
    {
        OutColour = 0,
        GBufferPosition,
        GBufferNormal,
        GBufferColour,
        GBufferMaterial
    };

	/// <summary>
	/// Enum to name texture slots as useful.
	/// </summary>
	enum TextureSlot
	{
		TEmpty = GL_TEXTURE0,
		TPosition,
		TNormal,
		TMaterial,
		TShadow,
		TInput,
		TArea,
		TSearch
	};

    /// <summary>
    /// Enum used for buffer block bindings to ensure they are consistent.
    /// </summary>
    enum UniformBufferId
    {
        Static = 0,
        Frame,
        Light,
        Shadow,
        Viewport
    };

    namespace Utils
    {
		/// <summary>
		/// Returns a texture ID based on texture location.
		/// </summary>
		GLuint getTextureID(TextureSlot texLocation);

		/// <summary>
		/// Bind the GBuffer textures for use.
		/// </summary>
		void bindGBufferTextures(GBuffer gbuffer);

		/// <summary>
		/// Unbind the GBuffer textures for drawing to.
		/// </summary>
		void unbindGBufferTextures();

		/// <summary>
		/// Set texture uniform with <id> in <programs> as <value>.
		/// </summary>
		void setTextureUniform(std::vector<GLuint> programs, const std::string& id, GLuint value);

		/// <summary>
		/// Set texture uniform with <id> in <program> as <value>.
		/// </summary>
		void setTextureUniform(GLuint program, const std::string& id, GLuint value);
		

        /// <summary>
        /// Create a framebuffer with <depthTex> depth attachment to be used as a shadowmap.
        /// </summary>
        ShadowMap createShadowMap(GLuint resolution);

        /// <summary>
        /// Resize the depthTex attachment of a givne shadow map.
        /// </summary>
        void resizeShadowMap(ShadowMap shadowMap, GLuint resolution);

		/// <summary>
		/// Create a framebuffer with <depth> depth attachment and RGBA8 colours.
		/// </summary>
		LBuffer createLBuffer(GLuint width, GLuint height, GLuint depth);

		/// <summary>
		/// Create a framebuffer with <depth> depth, RGB32F position, RGB32F normal and R8UI material attachments.
		/// </summary>
        GBuffer createGBuffer(GLuint width, GLuint height);

        /// <summary>
        /// Set height and width of GBuffer and LBuffer attachments, assuming a shared depth buffer.
        /// </summary>
        void resizeFramebuffers(GLuint width, GLuint height, GBuffer gbuffer, LBuffer lbuffer);

        /// <summary>
        /// Given a scene and aspect ratio outputs the ViewProjection matrix.
        /// </summary>
        glm::mat4 getViewProjectionMatrix(const sponza::Context& camera, float aspectRatio);

        /// <summary>
        /// Loads a given texture into an opengl texture buffer returning the id.
        /// </summary>
		GLuint loadTexture(const std::string& filename);

        /// <summary>
        /// Generates a buffer with the given data and usage.
        /// </summary>
        void genBuffer(GLuint& id, GLuint target, size_t bytes, void* data, GLuint usage = GL_STATIC_DRAW);
    }
}