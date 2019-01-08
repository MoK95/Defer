layout(std140) uniform PerFrameData
{
    MeshInstanceData Instances[100];
    mat4 ViewProjectionMatrix;
    vec3 EyePosition;
    int Padding0;
};

layout(std140) uniform ViewportData
{
    vec4 RTData;
};

uniform sampler2DRect Positions;
uniform sampler2DRect Normals;
uniform usampler2DRect MaterialIDs;

uniform sampler2D Input;
uniform sampler2D Search;

out vec4 OutColour;

bool outOfView(vec3 pos)
{
    bool value = pos.x < 0.0 || pos.x > 1.0 || pos.y < 0.0 || pos.y > 1.0 || pos.z < 0.0 || pos.z > 1.0;
    return value;
}

vec4 toScreenSpace(vec4 input)
{
	vec4 pos = ViewProjectionMatrix * input;
	pos /= pos.w;
	pos += 1.0;
	pos /= 2.0;

	return pos;
}

bool CheckPos(vec3 worldPos, vec2 posUV, out float l1, out float l2)
{
	l1 = length(worldPos - EyePosition);
	vec3 rayFragPos = texture(Positions, posUV * RTData.zw).xyz;
	l2 = length(rayFragPos - EyePosition);
	return l1 >= l2;
}

void main(void)
{
    uint M = texture(MaterialIDs, gl_FragCoord.xy).x;
	// Special material added for the sake of reflecting only on sponza's floor.
    if (M == 7u)
    {
		const float Gloss = 0.25f;
		float stepSize = 1;
		const int stepCount = 100;
		vec3 P = texture(Positions, gl_FragCoord.xy).xyz;
		vec3 N = texture(Normals, gl_FragCoord.xy).xyz;

        vec3 R = reflect(P - EyePosition, N);
        vec3 Rdir = normalize(R); 
		vec3 Pos = P;
        for (int i = 1; i < stepCount; ++i)
        {
			Pos += Rdir * stepSize;
			
			// Rasterise position to get UV to sample.
			vec4 rasterPos = toScreenSpace(vec4(Pos, 1.0));

            // If something leaves the frustum then we can't get a reflection
            if (outOfView(rasterPos.xyz)) break;

			float l1;
			float l2;

            // If depth is less than raster z we've hit something. 
            if (CheckPos(Pos, rasterPos.xy, l1, l2))
            {
				if (abs(l1 - l2) <= 0.5)
				{
					OutColour = texture(Input, rasterPos.xy) * Gloss;
					return;
				}
				else
				{
					stepSize *= -1.0f;
					// We are beyond the position but want to resolve a more accurate result via binary searching.
					for (int j = 0; j < stepCount; ++j)
					{
						stepSize *= 0.5f;
						Pos += Rdir * stepSize; 
						rasterPos = toScreenSpace(vec4(Pos, 1.0));

						if (CheckPos(Pos, rasterPos.xy, l1, l2))
						{
							stepSize = -abs(stepSize);
						}
						else
						{
							stepSize = abs(stepSize);
						}

						if (abs(l1 - l2) <= 0.5)
						{
							OutColour = texture(Input, rasterPos.xy) * Gloss;
							return;
						}
					}
					return;
				}
            }
			else
			{
				stepSize *= 1.25;
			}
        }
    }
}