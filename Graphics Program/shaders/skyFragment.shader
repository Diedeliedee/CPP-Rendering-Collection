#version 330 core
out vec4 FragColor;

in vec3	worldPosition;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

void main()
{
	//	Specular data
	vec3 viewDir	= normalize(worldPosition - cameraPosition);
	FragColor		= vec4(1, 0, 0, 1);
}