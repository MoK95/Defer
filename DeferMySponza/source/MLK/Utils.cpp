#include "Utils.hpp"

#include <iostream>
#include <tygra/FileHelper.hpp>
#include <sponza/Camera.hpp>
#include <sponza/Context.hpp>
#include <sponza/Mesh.hpp>
#include <sponza/GeometryBuilder.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace MLK
{
    namespace Utils
    {
        glm::mat4 getViewProjectionMatrix(const sponza::Context& scene, float aspectRatio)
        {
            const auto& camera = scene.getCamera();
            const auto& cameraPosition = (const glm::vec3&)camera.getPosition();
            const auto& cameraDirection = (const glm::vec3&)camera.getDirection();
            const auto& sceneUpDirection = (const glm::vec3&)scene.getUpDirection();

            const auto& viewMatrix = glm::lookAt(cameraPosition,
                cameraPosition + cameraDirection,
                sceneUpDirection);

            const auto& fovy = glm::radians(camera.getVerticalFieldOfViewInDegrees());
            const auto& near = camera.getNearPlaneDistance();
            const auto& far = camera.getFarPlaneDistance();

            const auto& projectionMatrix = glm::perspective(fovy, aspectRatio, near, far);

			const auto& VP = projectionMatrix * viewMatrix;

			return VP;
        }

		GLuint loadTexture(const std::string& filename)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			GLuint identifier = 0;

			tygra::Image texture_image = tygra::createImageFromPngFile(filename);
			if (texture_image.doesContainData())
			{
				glGenTextures(1, &identifier);
				glBindTexture(GL_TEXTURE_2D, identifier);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
				glTexImage2D(GL_TEXTURE_2D,
					0,
					GL_RGBA,
					(const GLsizei&)texture_image.width(),
                    (const GLsizei&)texture_image.height(),
					0,
					pixel_formats[texture_image.componentsPerPixel()],
					texture_image.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                    texture_image.pixelData());
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, identifier);
			return identifier;
		}

        ShadowMap createShadowMap(GLuint resolution)
        {
			// Understanding this should be a shadowSample and have the texture comparison parameters set,
			// I found that a normal texture actually looks nicer than the shadow sampler.
            ShadowMap shadowMap;

            glGenTextures(1, &shadowMap.depthTex);
            glBindTexture(GL_TEXTURE_2D, shadowMap.depthTex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

            glGenFramebuffers(1, &shadowMap.fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.fbo);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.depthTex, 0);

            auto success = glCheckFramebufferStatus(GL_FRAMEBUFFER);

            assert(success == GL_FRAMEBUFFER_COMPLETE);

            glDrawBuffer(GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);

            return shadowMap;
        }

        void resizeShadowMap(ShadowMap shadowMap, GLuint resolution)
        {
            glBindTexture(GL_TEXTURE_2D, shadowMap.depthTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

		LBuffer createLBuffer(GLuint width, GLuint height, GLuint depth)
		{
			LBuffer buffer;
			buffer.depth = depth;
			glGenFramebuffers(1, &buffer.fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);

			glGenTextures(1, &buffer.color);
			glBindTexture(GL_TEXTURE_2D, buffer.color);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_BYTE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.color, 0);

            glBindTexture(GL_TEXTURE_2D, depth);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

			auto success = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			assert(success == GL_FRAMEBUFFER_COMPLETE);
            
            glBindTexture(GL_TEXTURE_2D, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			return buffer;
		}

        GBuffer createGBuffer(GLuint width, GLuint height)
        {
            GBuffer buffer;
            glGenFramebuffers(1, &buffer.fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);

			GLuint attachments[3] =
			{
				GL_COLOR_ATTACHMENT0,
				GL_COLOR_ATTACHMENT1,
				GL_COLOR_ATTACHMENT2
			};

            glGenTextures(1, &buffer.posTex);
            glBindTexture(GL_TEXTURE_RECTANGLE, buffer.posTex);
            glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[0], GL_TEXTURE_RECTANGLE, buffer.posTex, 0);

            glGenTextures(1, &buffer.normTex);
            glBindTexture(GL_TEXTURE_RECTANGLE, buffer.normTex);

            glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[1], GL_TEXTURE_RECTANGLE, buffer.normTex, 0);

            glGenTextures(1, &buffer.matTex);
            glBindTexture(GL_TEXTURE_RECTANGLE, buffer.matTex);
            glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[2], GL_TEXTURE_RECTANGLE, buffer.matTex, 0);
			
            glGenTextures(1, &buffer.depth);
            glBindTexture(GL_TEXTURE_2D, buffer.depth);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, buffer.depth, 0);

			glDrawBuffers(3, attachments);

            auto success = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			assert(success == GL_FRAMEBUFFER_COMPLETE);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);

            return buffer;
        }

		void resizeFramebuffers(GLuint width, GLuint height, GBuffer gbuffer, LBuffer lbuffer)
		{
            glActiveTexture(TextureSlot::TEmpty);

			glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer.posTex);
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
			glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer.normTex);
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
			glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer.matTex);
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);

			glBindTexture(GL_TEXTURE_2D, lbuffer.color);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_BYTE, 0);

            glBindTexture(GL_TEXTURE_2D, gbuffer.depth);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

			assert(gbuffer.depth == lbuffer.depth);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
		}

		GLuint getTextureID(TextureSlot texLocation)
		{
			return texLocation - GL_TEXTURE0;
		}

		void bindGBufferTextures(GBuffer gbuffer)
		{
			glActiveTexture(TextureSlot::TPosition);
			glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer.posTex);
			glActiveTexture(TextureSlot::TNormal);
			glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer.normTex);
			glActiveTexture(TextureSlot::TMaterial);
			glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer.matTex);
			glActiveTexture(TextureSlot::TEmpty);
		}

		void unbindGBufferTextures()
		{
			glActiveTexture(TextureSlot::TPosition);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			glActiveTexture(TextureSlot::TNormal);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			glActiveTexture(TextureSlot::TMaterial);
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			glActiveTexture(TextureSlot::TEmpty);
		}

		void setTextureUniform(GLuint program, const std::string& id, GLuint value)
		{
			glUseProgram(program);
			auto location = glGetUniformLocation(program, id.c_str());
			glUniform1i(location, value);
			glUseProgram(0);
		}

		void setTextureUniform(std::vector<GLuint> programs, const std::string& id, GLuint value)
		{
			for (const auto program : programs)
			{
				setTextureUniform(program, id, value);
			}
		}

        void genBuffer(GLuint& id, GLuint target, size_t bytes, void* data, GLuint usage)
        {
            glGenBuffers(1, &id);
            glBindBuffer(target, id);
            glBufferData(target, bytes, data, usage);
            glBindBuffer(target, 0);
        }
    }
}