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

layout (std140) uniform ShadowData
{
    mat4 ShadowVP;
};

vec3 spotLight(Light light, vec3 P, vec3 N, uint M);
vec3 specularLight(const vec3 diffuseIntensity, const vec3 lightToPixel, vec3 P, vec3 N, uint M);

uniform sampler2DRect Positions;
uniform sampler2DRect Normals;
uniform usampler2DRect MaterialIDs;
uniform sampler2D ShadowMap;

out vec4 OutColour;

void main(void)
{
    vec3 P = texture(Positions, gl_FragCoord.xy).xyz;
    vec3 N = texture(Normals, gl_FragCoord.xy).xyz;
    uint M = texture(MaterialIDs, gl_FragCoord.xy).x;

	float visibility = 1.0f;

	vec4 lightSpacePos = ShadowVP * vec4(P, 1);
    // Sort bias.
	lightSpacePos = lightSpacePos / lightSpacePos.w;
	lightSpacePos = lightSpacePos / 2 + 0.5;
	if (texture(ShadowMap, lightSpacePos.xy).r < lightSpacePos.z)
	{
		visibility = 0.0f;
	}

	vec3 color = spotLight(light, P, N, M) * visibility;

	OutColour = vec4(color, 1.0);
}

vec3 spotLight(Light light, vec3 P, vec3 N, uint M)
{
	// Vector from pixel to light.
	vec3 L = light.Position - P;

    // Light scaling factors.
    float lightToPixelRange = length(L);
	float attenuation = 1.0 - smoothstep(0, light.Range, lightToPixelRange);

    L = normalize(L);

    float ratio = max(0, dot(L, N));

    // Smoothstep between inner and outer cone for fade on spotlight.
	float spotEffect = dot(L, -light.Direction);
	spotEffect = smoothstep(cos(light.Angle), cos(light.Angle / 1.5), spotEffect);
    
    float scalar = attenuation * ratio * spotEffect;

	vec3 diffuse = scalar * light.Intensity * materials[M].DiffuseColour;
    vec3 specular = specularLight(diffuse, -L, P, N, M) * materials[M].IsShiny;
	
    return (diffuse + specular);
}

vec3 specularLight(const vec3 intensity, const vec3 lightToPixel, vec3 P, vec3 N, uint M)
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
    return intensity * materials[M].SpecularColour * vec3(specFactor);
}
