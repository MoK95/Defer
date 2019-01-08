out vec3 GBufferPosition;
out vec3 GBufferNormal;
out uint GBufferMaterial;

in vec3 P;
in vec3 N;
in vec2 UV;
flat in uint MaterialId;

void main(void)
{
	gl_FragDepth = gl_FragCoord.z;
    GBufferPosition = P;
    GBufferNormal = normalize(N);
    GBufferMaterial = uint(MaterialId);
}