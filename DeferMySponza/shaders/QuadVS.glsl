#version 330

in vec2 Position;
in vec2 UV0;

void main(void)
{ 
	gl_Position = vec4(Position, 0.0, 1.0);
}