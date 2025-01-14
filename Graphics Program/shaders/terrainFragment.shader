#version 330 core
out vec4 FragColor;

in vec3	color;
in vec2	uv;
in mat3 tbn;
in vec3	worldPosition;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

void main()
{
	//	Normal calculation.
	vec3 normal	= texture(normalTex, uv).rgb;
	normal		= normalize(normal * 2.0 - 1.0);

	//	Adjusting normal strength.
	normal.rg	= normal.rg * 0.5f;
	normal		= normalize(normal);

	//	Calculating TBN.
	normal = tbn * normal;

	//	Specular data
	vec3 lightDirection	= normalize(worldPosition - lightPosition);
	vec3 viewDir		= normalize(worldPosition - cameraPosition);
	vec3 reflDir		= normalize(reflect(lightDirection, normal));
	
	//	Lighting
	float lightValue	= max(-dot(normal, lightDirection), 0.0);
	float specular		= pow(max(-dot(reflDir, viewDir), 0.0), 64);

	//	Separate RGB and alpha.
	//	Multiplying specular with color seems to be a mettalic property. More research required.
	vec4 output	= vec4(color, 1.0f) * texture(diffuseTex, uv);
	output.rgb	= output.rgb * min(lightValue + 0.1, 1.0) + specular * output.rgb;

	FragColor = output;
}