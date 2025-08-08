#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec2 ScreenCoords;

uniform sampler2D renderTexture;
uniform sampler2D testTexture;

void main()
{
	FragColor = texture(renderTexture, ScreenCoords);
}