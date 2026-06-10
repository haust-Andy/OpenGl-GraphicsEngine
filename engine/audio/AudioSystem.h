#pragma once

#include <string>
#include <memory>

// ===== 音频引擎 (轻量 SoLoud 封装) =====
// 如未集成 SoLoud, 提供 stub 实现

class AudioEngine
{
public:
    static void Init();
    static void Shutdown();
    static void Update();

    static bool IsInitialized() { return s_Initialized; }

private:
    static bool s_Initialized;
};

// 音频源组件
struct AudioSourceComponent
{
    std::string FilePath;
    bool PlayOnAwake   = false;
    bool Loop          = false;
    float Volume       = 1.0f;
    float Pitch        = 1.0f;
    float MinDistance   = 1.0f;
    float MaxDistance   = 100.0f;
    bool Spatial       = true;    // 3D 空间化
    bool Playing       = false;

    void Play();
    void Stop();
    void Pause();
    void SetVolume(float vol) { Volume = vol; }
    void SetPitch(float p) { Pitch = p; }
};

// 音频监听器组件 (跟随相机)
struct AudioListenerComponent
{
    float MasterVolume = 1.0f;
};
