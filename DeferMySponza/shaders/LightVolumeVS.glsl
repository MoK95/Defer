layout(std140) uniform PerFrameData
{
    MeshInstanceData Instances[100];
    mat4 ViewProjectionMatrix;
    vec3 EyePosition;
    int Padding0;
};

layout(std140) uniform LightData
{
	Light light;
};

in vec3 Position;

void main(void)
{
    // World position.
	gl_Position = ViewProjectionMatrix * light.ModelTransform * vec4(Position, 1.0);
}
