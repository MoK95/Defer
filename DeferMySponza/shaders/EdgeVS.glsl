in vec2 Position;
in vec2 UV0;

out vec2 texcoord;
out vec4 offset[3];

void main(void)
{
    texcoord = UV0;
    SMAAEdgeDetectionVS(texcoord, offset);
    gl_Position = vec4(Position, 0.0, 1.0);
}
