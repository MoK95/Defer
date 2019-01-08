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

layout (std140) uniform LightData
{
	Light light;
};

vec3 pointLight(const Light light, vec3 P, vec3 N, uint M);
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
	OutColour = vec4(pointLight(light, P, N, M), 1.0);
}

vec3 pointLight(const Light light, vec3 P, vec3 N, uint M)
{
	// Vector from pixel to light.
	vec3 L = light.Position - P;
	vec3 normalizedL = normalize(L);

    // Light scaling factors.
	float attenuation = (1.0 - smoothstep(0, light.Range, length(L)));
	float ratio = max(0, dot(normalizedL, N));
    float scalar = attenuation * ratio;


	vec3 diffuse = light.Intensity * scalar * materials[M].DiffuseColour;
	vec3 specular = specularLight(diffuse, -normalizedL, P, N, M) * materials[M].IsShiny;

	// Multiply by IsShiny which will be 0 if false, removing all specular, or 1 when true. Avoids branching.
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
