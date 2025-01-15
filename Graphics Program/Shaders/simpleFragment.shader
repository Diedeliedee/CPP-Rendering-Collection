#version 330 core
out vec4 FragColor;

in vec3	color;
in vec2	uv;
in mat3 tbn;
in vec3	worldPosition;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform vec3 lightColor;
uniform vec3 ambientLightColor;

uniform float time;

void main()
{
	//	Separate RGB and alpha.
	vec4 diffuse	= vec4(color, 1.0f) * texture(diffuseTex, uv);

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
	vec3 ambient	= ambientLightColor	* 0.25;
	vec3 light		= lightColor		* max(-dot(normal, lightDirection), 0.0);
	float specular	= texture(specularTex, uv).x * pow(max(-dot(reflDir, viewDir), 0.0), 512);

	vec4 output	= diffuse;
	output.rgb	*= ambient + light;
	output.rgb	+= specular;

	FragColor = output;
}