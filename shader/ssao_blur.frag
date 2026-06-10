#version 330 core
out vec4 FragColor;

in vec2 v_TexCoords;

uniform sampler2D u_SSAO;

void main()
{
    // 4x4 双边模糊
    vec2 texelSize = 1.0 / vec2(textureSize(u_SSAO, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_SSAO, v_TexCoords + offset).r;
        }
    }
    float ao = result / 16.0;
    FragColor = vec4(vec3(ao), 1.0);
}
