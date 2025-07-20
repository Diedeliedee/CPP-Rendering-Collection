#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

out vec2 uv;
out vec3 worldPosition;

uniform mat4 world, view, projection;

uniform sampler2D diffuseTex;

void main()
{
	//	Object space offset.
	vec3 pos = aPos;

	//	World space offset.
	vec4 worldPos = world * vec4(pos, 1.0);

	worldPos.y += texture(diffuseTex, vUV).r * 100.0f;

	gl_Position	= projection * view * worldPos;
	uv			= vUV;

	worldPosition = mat3(world) * aPos;
}