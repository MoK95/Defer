uniform sampler2D Input;
uniform sampler2D Area;
uniform sampler2D Search;

in vec2 texcoord;
in vec2 pixcoord;
in vec4 offset[3];

out vec4 OutColour;

void main()
{
    //OutColour = texture(Input, pixcoord);
    OutColour = SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, Input, Area, Search, ivec4(0));
}