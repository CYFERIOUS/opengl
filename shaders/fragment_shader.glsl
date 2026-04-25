#version 330 core

in vec4 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTex; // bound to unit 0 in your code

void main()
{
	vec4 tex = texture(uTex, vTexCoord);
	 FragColor = tex * vColor;
}
