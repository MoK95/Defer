layout(std140) uniform StaticData
{
	Material materials[16];
	GlobalLight GlobalLights;
};

layout(std140) uniform PerFrameData
{
	MeshInstanceData Instances[100];
	mat4 ViewProjectionMatrix;
	vec3 EyePosition;
	int Padding0;
};

vec3 directionalLight(DirectionalLight light, vec3 P, vec3 N, uint M);
vec3 specularLight(const vec3 diffuseIntensity, const vec3 lightToPixel, vec3 P, vec3 N, uint M);

uniform sampler2DRect Positions;
uniform sampler2DRect Normals;
uniform usampler2DRect MaterialIDs;

out vec4 OutColour;

void main(void)
{
	vec3 P = texture(Positions, gl_FragCoord.xy).xyz;
	vec3 N = texture(Normals, gl_FragCoord.xy).xyz;
	uint M = texture(MaterialIDs, gl_FragCoord.xy).x;

	vec3 ambient = GlobalLights.AmbientIntensity * materials[M].DiffuseColour;

	vec3 directional = vec3(0, 0, 0);
	for (int i = 0; i < GlobalLights.DirectionalLightCount; i++)
	{
		directional += directionalLight(GlobalLights.DirectionalLights[i], P, N, M);
	}

	OutColour = vec4(directional + ambient, 1.0);
}

vec3 directionalLight(DirectionalLight light, vec3 P, vec3 N, uint M)
{
	// Pixel to light vector.
	vec3 pixelToLight = -light.Direction;

	// Check angle between surface normal and light direction to use as a scalar.
	float cosAngle = max(0, dot(normalize(pixelToLight), N));

	// Calculate diffues light dependant on color, intensity and angle scalar.
	vec3 diffuse = materials[M].DiffuseColour * light.Intensity * cosAngle;

	// Calculate specular light, IsShiny being 0 or 1.
	vec3 specular = specularLight(diffuse, light.Direction, P, N, M) * materials[M].IsShiny;

	// Return combined light.
	return  diffuse + specular;
}

vec3 specularLight(const vec3 diffuseIntensity, const vec3 lightToPixel, vec3 P, vec3 N, uint M)
{
	// Required Reflection and Eye Direction vectors.
	vec3 R = normalize(reflect(lightToPixel, N));
	vec3 V = normalize(EyePosition - P);

	// Angle between eye and reflected vector.
	float angle = dot(V, R);

	// Ensure positive angle.
	float specFactor = max(0, angle);

	// Multiply by shininess and clamp between 0 an 1.
	specFactor = pow(specFactor, materials[M].Shininess);
	specFactor = clamp(specFactor, 0, 1);

	// Return specular.
	return diffuseIntensity * materials[M].SpecularColour * vec3(specFactor);
}