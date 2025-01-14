#version 330 core
out vec4 FragColor;

in vec2	uv;
in vec3	normal;
in vec3	worldPosition;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

void main()
{
	//	Normal calculation.
	//vec3 normal	= texture(normalTex, uv).rgb;
	//normal		= normalize(normal * 2.0 - 1.0);

	//	Specular data
	//vec3 viewDir		= normalize(worldPosition - cameraPosition);
	//vec3 reflDir		= normalize(reflect(lightDirection, normal));
	
	//	Lighting
	float lightValue	= max(-dot(normal, lightDirection), 0.0);
	//float specular		= pow(max(-dot(reflDir, viewDir), 0.0), 64);

	//	Separate RGB and alpha.
	vec4 output	= texture(diffuseTex, uv);
	output.rgb	= output.rgb * min(lightValue + 0.1, 1.0); // + specular * output.rgb;

	FragColor = output;
}