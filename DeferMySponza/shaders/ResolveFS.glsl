uniform sampler2D Input;
uniform sampler2D Search;

in vec2 texcoord;
in vec4 offset;

out vec4 OutColour;

void main()
{
    OutColour = SMAANeighborhoodBlendingPS(texcoord, offset, Input, Search);
}