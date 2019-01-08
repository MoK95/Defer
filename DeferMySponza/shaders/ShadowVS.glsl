layout(std140) uniform PerFrameData
{
	MeshInstanceData Instances[100];
	mat4 ViewProjectionMatrix;
	vec3 EyePosition;
	int Padding0;
};

layout(std140) uniform ShadowData
{
    mat4 ShadowVP;
};

in vec3 Position;
in vec3 Normal;
in vec2 UV0;
in uint InstanceID;

void main(void)
{
	// World position.
	gl_Position = ShadowVP * Instances[InstanceID].ModelTransform * vec4(Position, 1.0);
}
