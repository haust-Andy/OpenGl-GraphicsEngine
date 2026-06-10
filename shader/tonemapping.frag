#version 330 core
out vec4 FragColor;
in vec2 v_TexCoords;

uniform sampler2D u_HDRTexture;
uniform float u_Exposure;
uniform float u_Gamma;

// ACES 电影级色调映射
vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdrColor = texture(u_HDRTexture, v_TexCoords).rgb;

    // 曝光调整
    hdrColor *= u_Exposure;

    // ACES 色调映射
    vec3 mapped = ACESFilm(hdrColor);

    // Gamma 校正
    mapped = pow(mapped, vec3(1.0 / u_Gamma));

    FragColor = vec4(mapped, 1.0);
}
