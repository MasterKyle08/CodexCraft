#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in uint aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in uint aLight;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

out VS_OUT
{
    vec3 normal;
    vec2 uv;
    float light;
} vs;

vec3 decodeNormal(uint packed)
{
    vec3 n;
    n.x = float(packed & 1023u) / 1023.0 * 2.0 - 1.0;
    n.y = float((packed >> 10) & 1023u) / 1023.0 * 2.0 - 1.0;
    n.z = float((packed >> 20) & 1023u) / 1023.0 * 2.0 - 1.0;
    return normalize(n);
}

void main()
{
    vec3 worldPos = vec3(uModel * vec4(aPosition, 1.0));
    gl_Position = uProjection * uView * vec4(worldPos, 1.0);

    mat3 normalMatrix = mat3(uModel);
    vs.normal = normalize(normalMatrix * decodeNormal(aNormal));
    vs.uv = aUV;
    vs.light = float(aLight) / 255.0;
}
