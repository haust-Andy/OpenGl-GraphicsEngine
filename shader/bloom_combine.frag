#version 330 core
out vec4 FragColor;
in vec2 v_TexCoords;

uniform sampler2D u_SceneTexture;
uniform sampler2D u_BloomTexture;
uniform float u_Intensity;

void main()
{
    vec3 sceneColor = texture(u_SceneTexture, v_TexCoords).rgb;
    vec3 bloomColor = texture(u_BloomTexture, v_TexCoords).rgb;

    FragColor = vec4(sceneColor + bloomColor * u_Intensity, 1.0);
}
