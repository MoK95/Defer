#pragma once

#include <tgl/tgl.h>
#include <glm/glm.hpp>

namespace sponza
{
    class DirectionalLight;
    class PointLight;
    class SpotLight;
    class Material;
}

namespace MLK
{
    /// <summary>
    /// Structure to store material data. 16 byte alligned to allow for easy use within shaders and GLSL arrays.
    /// </summary>
    struct ShaderMaterial
    {
        ShaderMaterial() {}

        ShaderMaterial(const sponza::Material& material);

        glm::vec3 DiffuseColour; // 12
        float Shininess = 0.f; // 16
        glm::vec3 SpecularColour; // 28
        int IsShiny; // 32
    };

    /// <summary>
    /// Structure to store light data. 16 byte alligned to allow for easy use within shaders and GLSL arrays.
    /// </summary>
	struct ShaderLight
	{
        ShaderLight() {}
		ShaderLight(const sponza::SpotLight& spotLight);
		ShaderLight(const sponza::PointLight& pointLight);

		glm::vec3 Position;
		float Range;
		glm::vec3 Intensity;
		float Angle;
		glm::vec3 Direction;
		GLint CastsShadows;
		glm::mat4 ModelTransform;
	};

	struct DirectionalLight
	{
		DirectionalLight() {}
		DirectionalLight(const sponza::DirectionalLight& directionalLight);

		glm::vec3 Direction;
		float Padding0;
		glm::vec3 Intensity;
		float Padding1;
	};

	struct GlobalLight
	{
		GlobalLight() {}

		glm::vec3 AmbientIntensity;
		GLint DirectionalLightCount;
		DirectionalLight DirectionalLights[2];
	};

    /// <summary>
    /// Structure to store mesh instance data. 16 byte alligned to allow for easy use within shaders and GLSL arrays.
    /// </summary>
	struct MeshInstanceData
	{
        MeshInstanceData() {}

		MeshInstanceData(const glm::mat4& modelTransform, GLuint materialId) :
			ModelTransform(modelTransform), MaterialIndex(materialId) {}

		glm::mat4 ModelTransform;
		GLuint MaterialIndex;
		GLuint Padding[3];
	};

    /// <summary>
    /// Structure for per frame data. Instance data, view projection matrix and eye position are expected to change once per frame (for now).
    /// </summary>
    struct PerFrameUniformData
    {
        MeshInstanceData InstanceData[100];
        glm::mat4 ViewProjectionMatrix;
        glm::vec3 EyePosition;
        GLuint Padding;
    };

    /// <summary>
    /// Structure for static uniform data. Expected that materials and ambient light values will not change throughout scene duration.
    /// </summary>
    struct StaticUniformData
    {
        ShaderMaterial Materials[16];
		GlobalLight GlobalLights;
    };

    struct ShadowUniform
    {
        glm::mat4 VP;
    };

    struct ViewportData
    {
        float PixelWidth;
        float PixelHeight;
        float ScreenWidth;
        float ScreenHeight;
    };
}