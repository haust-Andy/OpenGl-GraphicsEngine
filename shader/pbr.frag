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

struct SpotLight {
    vec3 Position;
    vec3 Direction;
    vec3 Color;
    float Intensity;
    float Range;
    float InnerCutOff;
    float OuterCutOff;
};
uniform int u_SpotLightCount;
uniform SpotLight u_SpotLights[4];

// ===== 阴影 =====
uniform bool u_ShadowsEnabled;
uniform sampler2DArray u_ShadowMap;
uniform int   u_CascadeCount;
uniform float u_ShadowBias;
uniform float u_ShadowNormalBias;
uniform bool  u_SoftShadows;
uniform int   u_PCFSamples;
uniform float u_PCFRadius;

struct CascadeInfo {
    mat4 LightViewProjection;
    float SplitDepth;
};
uniform CascadeInfo u_Cascades[4];

// ===== IBL =====
uniform bool u_IBL_Enabled;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;

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

// ===== 阴影计算 =====
float CalculateShadow(vec3 worldPos, vec3 normal, vec3 lightDir)
{
    if (!u_ShadowsEnabled) return 0.0;

    // 选择级联
    int cascadeIndex = 0;
    for (int i = 0; i < u_CascadeCount; i++)
    {
        if (length(worldPos - u_CameraPos) < u_Cascades[i].SplitDepth)
        {
            cascadeIndex = i;
            break;
        }
    }

    // 变换到光空间
    vec4 lightSpacePos = u_Cascades[cascadeIndex].LightViewProjection * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // 超出阴影范围
    if (projCoords.z > 1.0 || projCoords.z < 0.0) return 0.0;

    // 法线偏移 (减少 Shadow Acne) - 显式取 xy 分量
    float bias = max(u_ShadowBias * (1.0 - dot(normal, lightDir)), u_ShadowBias);
    projCoords.xy += normal.xy * u_ShadowNormalBias;

    float shadow = 0.0;

    if (u_SoftShadows)
    {
        // M-04: Poisson Disk 采样替代固定网格，采样数量与空间分布一致
        vec2 texelSize = 1.0 / vec2(textureSize(u_ShadowMap, 0));
        float radius = u_PCFRadius;
        int samples = u_PCFSamples;

        // 预计算的 Poisson Disk 偏移 (32个采样点)
        const vec2 poissonDisk[32] = vec2[](
            vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
            vec2(-0.09418410, -0.92938870), vec2(0.34495938, 0.29387760),
            vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
            vec2(-0.38277543, 0.27676846), vec2(0.97484398, 0.75648379),
            vec2(0.44323322, -0.97506874), vec2(0.53742981, -0.47373420),
            vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19068188),
            vec2(-0.24188840, 0.99206618), vec2(-0.47331357, -0.71857792),
            vec2(0.34702856, 0.85437883), vec2(0.69096497, -0.46706450),
            vec2(-0.66086262, 0.49440350), vec2(0.15098090, -0.55374140),
            vec2(-0.69801931, -0.16493970), vec2(0.83939329, 0.21779500),
            vec2(-0.27756790, 0.87862560), vec2(0.32024570, 0.63012030),
            vec2(-0.40632750, -0.15700410), vec2(0.10422230, 0.41083060),
            vec2(-0.81748380, 0.11245250), vec2(0.78096270, 0.41927370),
            vec2(-0.53083480, 0.75708730), vec2(0.58368150, 0.60126490),
            vec2(-0.11964640, -0.70543900), vec2(0.24245900, -0.10839950),
            vec2(-0.87373990, -0.48424680), vec2(0.47056290, -0.26541130)
        );

        int actualSamples = min(samples, 32);
        for (int i = 0; i < actualSamples; i++)
        {
            vec2 offset = poissonDisk[i] * texelSize * radius;
            float depth = texture(u_ShadowMap, vec3(projCoords.xy + offset, float(cascadeIndex))).r;
            shadow += (projCoords.z - bias) > depth ? 1.0 : 0.0;
        }
        shadow /= float(actualSamples);
    }
    else
    {
        float depth = texture(u_ShadowMap, vec3(projCoords.xy, float(cascadeIndex))).r;
        shadow = (projCoords.z - bias) > depth ? 1.0 : 0.0;
    }

    return shadow;
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

    // == Directional Light + Shadow ==
    vec3 L = normalize(-u_DirectionalLight.Direction);
    vec3 H = normalize(V + L);
    vec3 radiance = u_DirectionalLight.Color * u_DirectionalLight.Intensity;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    float shadow = CalculateShadow(v_WorldPos, N, L);
    Lo += (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0) * (1.0 - shadow);

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

    // == Spot Lights ==
    for (int i = 0; i < u_SpotLightCount && i < 4; i++)
    {
        vec3 lightDir = u_SpotLights[i].Position - v_WorldPos;
        float distance = length(lightDir);
        if (distance > u_SpotLights[i].Range) continue;

        lightDir = normalize(lightDir);
        vec3 spotDir = normalize(u_SpotLights[i].Direction);
        float theta = dot(lightDir, -spotDir);
        float innerCos  = cos(radians(u_SpotLights[i].InnerCutOff));
        float outerCos  = cos(radians(u_SpotLights[i].OuterCutOff));
        float epsilon = innerCos - outerCos;
        float intensity = clamp((theta - outerCos) / epsilon, 0.0, 1.0);
        if (intensity <= 0.0) continue;

        L = normalize(lightDir);
        H = normalize(V + L);
        float attenuation = 1.0 / (distance * distance);
        radiance = u_SpotLights[i].Color * u_SpotLights[i].Intensity * attenuation * intensity;

        NDF = DistributionGGX(N, H, roughness);
        G   = GeometrySmith(N, V, L, roughness);
        F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
        numerator = NDF * G * F;
        denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        specular = numerator / denominator;

        kD = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);
    }

    vec3 ambient = u_AmbientLight * albedo * ao;

    // IBL 环境光照
    if (u_IBL_Enabled)
    {
        // 漫反射 IBL: 辐照度图
        vec3 kS = FresnelSchlick(max(dot(N, V), 0.0), F0);
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        vec3 diffuseIBL = irradiance * albedo;

        // 镜面反射 IBL: 预过滤环境图 + BRDF LUT
        vec3 R = reflect(-V, N);
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specularIBL = prefilteredColor * (kS * brdf.x + brdf.y);

        ambient = (kD * diffuseIBL + specularIBL) * ao;
    }

    vec3 color = ambient + Lo + emission;

    // 注意: Tone Mapping 和 Gamma 校正由后处理管线 (ToneMappingPass) 统一执行
    // 此处输出线性 HDR 值，确保 Bloom 等后处理效果正确工作

    FragColor = vec4(color, 1.0);
}
