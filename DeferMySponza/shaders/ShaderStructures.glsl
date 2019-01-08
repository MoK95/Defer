#version 330

// Alligned structure for data concerning a single mesh instance.
struct MeshInstanceData
{
	mat4 ModelTransform;
	uint MaterialIndex;
	int Padding1;
	int Padding2;
	int Padding3;
};

// Alligned structure for material data.
struct Material
{
    vec3 DiffuseColour;
    float Shininess;
    vec3 SpecularColour;
    int IsShiny;
};

struct DirectionalLight
{
	vec3 Direction;
	float Padding0;
	vec3 Intensity;
	float Padding1;
};

struct GlobalLight
{
	vec3 AmbientIntensity;
	int DirectionalLightCount;
	DirectionalLight DirectionalLights[2];
};

// Alligned structure representing a light.
struct Light
{
    vec3 Position;
    float Range;
    vec3 Intensity;
    float Angle;
    vec3 Direction;
    int CastsShadows;
	mat4 ModelTransform;
};