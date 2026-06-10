#version 330 core
out vec4 FragColor;

in vec2 v_TexCoords;

uniform sampler2D u_SceneColor;
uniform sampler2D u_SSAO;

void main()
{
    vec3 color = texture(u_SceneColor, v_TexCoords).rgb;
    float ao   = texture(u_SSAO, v_TexCoords).r;

    // AO 乘到环境光项
    FragColor = vec4(color * ao, 1.0);
}
