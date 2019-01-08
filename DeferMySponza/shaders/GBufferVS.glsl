layout(std140) uniform PerFrameData
{
    MeshInstanceData Instances[100];
    mat4 ViewProjectionMatrix;
    vec3 EyePosition;
    int Padding0;
};

in vec3 Position;
in vec3 Normal;
in vec2 UV0;
in uint InstanceID;

out vec3 P;
out vec3 N;
out vec2 UV;
flat out uint MaterialId;

void main(void)
{ 
	MeshInstanceData instance = Instances[InstanceID];

	// Instance based positions.
	P = (instance.ModelTransform * vec4(Position, 1.0)).xyz;

	// Instance based world normals.
	N = (normalize(instance.ModelTransform * vec4(Normal, 0))).xyz;

    // UVs.
    UV = UV0;

    // Material ID.
	MaterialId = instance.MaterialIndex;

    // World position.
	gl_Position = ViewProjectionMatrix * instance.ModelTransform * vec4(Position, 1.0);
}
