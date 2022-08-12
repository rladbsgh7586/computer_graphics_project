#version 330

// input from vertex shader
in vec2 tc;

// the only output variable
out vec4 fragColor;

// texture sampler
uniform sampler2D TEX;
uniform vec4 color;

void main()
{
	
	fragColor = texture2D( TEX, tc ) * color;
}
