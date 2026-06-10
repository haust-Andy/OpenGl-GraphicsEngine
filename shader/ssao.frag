#version 330 core
out vec4 FragColor;

in vec2 v_TexCoords;

uniform sampler2D u_Position;   // World space position
uniform sampler2D u_Normal;     // World space normal
uniform sampler2D u_Noise;      // Random noise texture

uniform vec3 u_Samples[64];    // Hemisphere samples
uniform mat4 u_Projection;
uniform mat4 u_View;

uniform vec2 u_ScreenSize;
uniform float u_Radius = 0.5;
uniform float u_Bias = 0.025;
uniform int u_KernelSize = 64;

void main()
{
    vec2 noiseScale = u_ScreenSize / 4.0;
    vec3 fragPos   = texture(u_Position, v_TexCoords).rgb;
    vec3 fragNormal = normalize(texture(u_Normal, v_TexCoords).rgb);
    vec3 randomVec  = normalize(texture(u_Noise, v_TexCoords * noiseScale).rgb);

    // TBN: 从法线和随机向量构建
    vec3 tangent   = normalize(randomVec - fragNormal * dot(randomVec, fragNormal));
    vec3 bitangent = cross(fragNormal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, fragNormal);

    float occlusion = 0.0;
    for (int i = 0; i < u_KernelSize; ++i)
    {
        // 采样点转世界空间
        vec3 samplePos = TBN * u_Samples[i];
        samplePos = fragPos + samplePos * u_Radius;

        // 投影到屏幕空间
        vec4 offset = u_Projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy  = offset.xy * 0.5 + 0.5;

        // 采样场景深度
        float sampleDepth = texture(u_Position, offset.xy).z;

        // Range check 防止远距离伪影
        float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + u_Bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(u_KernelSize));
    FragColor = vec4(vec3(occlusion), 1.0);
}
