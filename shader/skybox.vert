#version 330 core
layout (location = 0) in vec3 a_Position;

out vec3 v_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    // 移除平移分量, 天空盒始终以相机为中心
    mat4 viewNoTranslation = mat4(mat3(u_View));
    vec4 pos = u_Projection * viewNoTranslation * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;  // 确保深度值为1.0 (最远)
    v_TexCoords = a_Position;
}
