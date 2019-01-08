out vec4 OutColour;

in vec2 texcoord;
in vec4 offset[3];

uniform sampler2D Input;

void main(void)
{
    // Trial and error to find best edge detection for scenario, both functional.
    //vec2 outEdge = SMAALumaEdgeDetectionPS(texcoord, offset, Input).xy;
    vec2 outEdge = SMAAColorEdgeDetectionPS(texcoord, offset, Input).xy;
    OutColour = vec4(outEdge, 0.0, 0.0);
}