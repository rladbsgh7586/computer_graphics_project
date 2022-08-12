#version 330

// vertex attributes
in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec2 tc;	// texture coordinate

// uniforms
uniform vec2 center;
uniform vec2 scale;
uniform float aspect_ratio;

void main()
{
	vec2 pos ;
	pos.x = position.x * scale.x;
	pos.y = position.y * scale.y;
	//pos.xy *= aspect_ratio>1 ? vec2(1/aspect_ratio,1) : vec2(1,aspect_ratio); // tricky aspect correction

	gl_Position = vec4(center, 0, 0) +  vec4(pos, 0, 1);
	tc = texcoord;
}