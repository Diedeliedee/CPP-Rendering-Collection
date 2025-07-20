#version 330 core
out vec4 FragColor;

in vec2	uv;
in vec3	worldPosition;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

uniform sampler2D dirt, sand, grass, rock, snow;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

vec3 lerp(vec3 a, vec3 b, float t)
{
	return a + (b - a) * t;
}

void main()
{
	//	Normal calculation.
	vec3 normal	= texture(normalTex, uv).rgb;
	normal		= normalize(normal * 2.0 - 1.0);
	normal.gb	= normal.bg;						//	Flip normal channels because of conventions
	normal.r	= -normal.r;						//	Flips red axis.
	normal.b	= -normal.b;						//	Flips blue axis.

	//	Specular data
	//vec3 viewDir		= normalize(worldPosition - cameraPosition);
	//vec3 reflDir		= normalize(reflect(lightDirection, normal));
	
	//	Lighting
	float lightValue	= max(-dot(normal, lightDirection), 0.0);
	//float specular		= pow(max(-dot(reflDir, viewDir), 0.0), 64);

	//	Build color!
	float y = worldPosition.y;

	float ds = clamp((y - 50) / 10, -1, 1) * .5 + .5;
	float sg = clamp((y - 100) / 10, -1, 1) * .5 + .5;
	float gr = clamp((y - 150) / 10, -1, 1) * .5 + .5;
	float rs = clamp((y - 200) / 10, -1, 1) * .5 + .5;

	vec3 dirtColor	= texture(dirt, uv * 10).rgb;
	vec3 sandColor	= texture(sand, uv * 10).rgb;
	vec3 grassColor	= texture(grass, uv * 10).rgb;
	vec3 rockColor	= texture(rock, uv * 10).rgb;
	vec3 snowColor	= texture(snow, uv * 10).rgb;

	vec3 diffuse;
	diffuse = lerp(dirtColor, sandColor, ds);
	diffuse = lerp(diffuse, grassColor, sg);
	diffuse = lerp(diffuse, rockColor, gr);
	diffuse = lerp(diffuse, snowColor, rs);

	//	Separate RGB and alpha.
	vec4 _output = vec4(diffuse * min(lightValue + 0.1, 1.0), 1.0); // + specular * _output.rgb;

	FragColor = _output;
}