#version 330 core
out vec4 FragColor;

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoords;

// Material
struct MaterialData {
    vec3  Albedo;
    float Metallic;
    float Roughness;
    float AO;
    vec3  Emission;
};

uniform MaterialData u_Material;

// Texture maps
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;
uniform sampler2D u_EmissiveMap;

uniform bool u_HasAlbedoMap;
uniform bool u_HasNormalMap;
uniform bool u_HasMetallicMap;
uniform bool u_HasRoughnessMap;
uniform bool u_HasAOMap;
uniform bool u_HasEmissiveMap;

// Camera
uniform vec3 u_CameraPos;

// Lights
uniform vec3 u_AmbientLight;

struct DirectionalLight {
    vec3 Direction;
    vec3 Color;
    float Intensity;
};
uniform DirectionalLight u_DirectionalLight;

#define MAX_POINT_LIGHTS 16
struct PointLight {
    vec3 Position;
    vec3 Color;
    float Intensity;
    float Range;
    float Constant;
    float Linear;
    float Quadratic;
};
uniform int u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

// Constants
const float PI = 3.14159265359;

// PBR functions
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

void main()
{
    // Sample textures
    vec3 albedo = u_HasAlbedoMap
        ? pow(texture(u_AlbedoMap, v_TexCoords).rgb, vec3(2.2))
        : u_Material.Albedo;

    float metallic  = u_HasMetallicMap
        ? texture(u_MetallicMap, v_TexCoords).r
        : u_Material.Metallic;

    float roughness = u_HasRoughnessMap
        ? texture(u_RoughnessMap, v_TexCoords).r
        : u_Material.Roughness;

    float ao = u_HasAOMap
        ? texture(u_AOMap, v_TexCoords).r
        : u_Material.AO;

    vec3 emission = u_HasEmissiveMap
        ? texture(u_EmissiveMap, v_TexCoords).rgb
        : u_Material.Emission;

    vec3 N = normalize(v_Normal);
    vec3 V = normalize(u_CameraPos - v_WorldPos);

    // F0: 基础反射率 (金属用albedo, 非金属用0.04)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Reflectance equation
    vec3 Lo = vec3(0.0);

    // == Directional Light ==
    vec3 L = normalize(-u_DirectionalLight.Direction);
    vec3 H = normalize(V + L);
    float distance = 1.0;
    vec3 radiance = u_DirectionalLight.Color * u_DirectionalLight.Intensity;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    Lo += (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);

    // == Point Lights ==
    for (int i = 0; i < u_PointLightCount; i++)
    {
        vec3 lightDir = u_PointLights[i].Position - v_WorldPos;
        float distance = length(lightDir);
        if (distance > u_PointLights[i].Range) continue;

        L = normalize(lightDir);
        H = normalize(V + L);

        float attenuation = 1.0 / (u_PointLights[i].Constant
            + u_PointLights[i].Linear * distance
            + u_PointLights[i].Quadratic * distance * distance);

        radiance = u_PointLights[i].Color * u_PointLights[i].Intensity * attenuation;

        NDF = DistributionGGX(N, H, roughness);
        G = GeometrySmith(N, V, L, roughness);
        F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        numerator = NDF * G * F;
        denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        specular = numerator / denominator;

        kD = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);
    }

    vec3 ambient = u_AmbientLight * albedo * ao;
    vec3 color = ambient + Lo + emission;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
