#version 450 core

in VS_OUT
{
    vec3 normal;
    vec2 uv;
    float light;
} fs;

uniform sampler2D uAtlas;
uniform vec3 uLightDir;

out vec4 FragColor;

void main()
{
    vec3 lightDir = normalize(-uLightDir);
    float nDotL = max(dot(normalize(fs.normal), lightDir), 0.1);
    float shading = nDotL * fs.light;
    vec4 albedo = texture(uAtlas, fs.uv);
    FragColor = vec4(albedo.rgb * shading, albedo.a);
}
