#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

vec4 lerp(vec4 a, vec4 b, float t)
{
    return a + (b - a) * t;
}

vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void clip(float x)
{
    if (x < 0) discard;
}

void main()
{
    //  Sampling diffuse and specular textures.
    vec4 diffuse = texture(texture_diffuse1, TexCoords);
    vec4 specTex = texture(texture_specular1, TexCoords);

    //  Getting fragment light value.
    float light = max(dot(-lightDirection, Normals), 0.0);

    //  Calculating fragmet light reflection.
    vec3 viewDir = normalize(FragPos.rgb - cameraPosition);
    vec3 refl = reflect(lightDirection, Normals);

    //  Sampling AO texture.
    float ambientOcclusion = texture(texture_ao1, TexCoords).r;
    
    //  Applying reflections.
    float roughness = texture(texture_roughness1, TexCoords).r;
    float spec      = pow(max(dot(-viewDir, refl), 0.0), lerp(1, 128, roughness));
    vec3 specular   = spec * specTex.rgb;

    //  Applying fog.
    float dist      = length(FragPos.xyz - cameraPosition);
    float fog       = 1;
    float density   = 0.001;

    fog = 1 / pow(2, pow(dist * density, 2));   //	Calculate fragment fog.
    fog = 1 - fog;                              //	Inverting.

    vec3 topColor = vec3(68.0 / 255.0, 118.0 / 255.0, 189.0 / 255.0);
    vec3 botColor = vec3(188.0 / 255.0, 214.0 / 255.0, 231.0 / 255.0);
    
    vec3 fogColor = lerp(botColor, topColor, max(viewDir.y, 0.0));

    //  Constructing output.
    vec4 _output = lerp(diffuse * max(light * ambientOcclusion, 0.2 * ambientOcclusion) + vec4(specular, 0), vec4(fogColor, 1.0), fog);
    
     // Treshold clipping.
     //if (_output.a < 0.5) discard;

    //  Output result.
    FragColor = _output;
}