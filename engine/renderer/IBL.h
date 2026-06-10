#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

// IBL (Image-Based Lighting) 环境光照
// 负责 HDR 环境贴图加载、辐照度图卷积、预过滤环境图、BRDF LUT
class IBL
{
public:
    IBL();
    ~IBL();

    // 从 HDR 文件加载环境贴图并预计算所有 IBL 资源
    bool LoadFromHDR(const std::string& path);

    // 从程序化天空盒生成 (无 HDR 文件时)
    bool GenerateFromProcedural(const glm::vec3& skyTopColor = glm::vec3(0.3f, 0.5f, 1.0f),
                                 const glm::vec3& skyBottomColor = glm::vec3(0.05f, 0.05f, 0.15f));

    // 绑定 IBL 资源到着色器
    void BindIrradianceMap(uint32_t slot) const;
    void BindPrefilterMap(uint32_t slot) const;
    void BindBRDFLUT(uint32_t slot) const;

    // 绑定环境贴图作为天空盒
    void BindEnvironmentMap(uint32_t slot) const;

    uint32_t GetIrradianceMapID() const { return m_IrradianceMap; }
    uint32_t GetPrefilterMapID()   const { return m_PrefilterMap; }
    uint32_t GetBRDFLUTID()       const { return m_BRDFLUT; }
    uint32_t GetEnvironmentMapID() const { return m_EnvironmentMap; }

    bool IsLoaded() const { return m_Loaded; }

    static constexpr uint32_t IRRADIANCE_SIZE = 32;
    static constexpr uint32_t PREFILTER_SIZE  = 128;
    static constexpr uint32_t BRDF_LUT_SIZE   = 512;
    static constexpr uint32_t ENV_MAP_SIZE    = 512;
    static constexpr uint32_t PREFILTER_MIP_LEVELS = 5;

private:
    void ConvertEquirectangularToCubeMap(uint32_t equirectTexture);
    void ConvolveIrradiance();
    void PrefilterEnvironment();
    void ComputeBRDFLUT();
    void RenderCubeMapFace(uint32_t fbo, uint32_t textureID, int face,
                            const glm::mat4& view, const glm::mat4& proj,
                            uint32_t mipLevel = 0, uint32_t mipSize = 0);

    uint32_t m_EnvironmentMap = 0;   // HDR 环境立方体贴图
    uint32_t m_IrradianceMap  = 0;   // 辐照度图 (漫反射 IBL)
    uint32_t m_PrefilterMap   = 0;   // 预过滤环境图 (镜面反射 IBL)
    uint32_t m_BRDFLUT        = 0;   // BRDF 积分 LUT

    std::shared_ptr<class Shader> m_EquirectToCubeShader;
    std::shared_ptr<class Shader> m_IrradianceShader;
    std::shared_ptr<class Shader> m_PrefilterShader;
    std::shared_ptr<class Shader> m_BRDFShader;

    bool m_Loaded = false;

    // 立方体贴图渲染用的 VAO
    uint32_t m_CubeVAO = 0;
    uint32_t m_CubeVBO = 0;
    uint32_t m_QuadVAO = 0;
    uint32_t m_QuadVBO = 0;
};
