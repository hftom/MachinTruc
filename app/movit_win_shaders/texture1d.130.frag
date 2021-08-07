#version 130

uniform sampler2D tex;
in vec2 tc;

out vec4 FragColor;

void main()
{
	FragColor = texture(tex, tc);  // Second component is irrelevant.
}
