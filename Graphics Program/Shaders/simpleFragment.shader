#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;
in vec3 worldPosition;

uniform sampler2D diffuse;
uniform vec3 lightPosition;

void main()
{
	//	Lighting
	vec3 lightDirection	= normalize(worldPosition - lightPosition);
	float lightValue	= max(-dot(normal, lightDirection), 0.0);

	FragColor = /*vec4(uv, 0.0f, 1.0f);*/	vec4(color, 1.0f) * texture(diffuse, uv) * min(lightValue + 0.1, 1.0);
}