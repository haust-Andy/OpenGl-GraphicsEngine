#include "AudioSystem.h"
#include <iostream>

bool AudioEngine::s_Initialized = false;

void AudioEngine::Init()
{
    if (s_Initialized) return;

    // TODO: 初始化 SoLoud 引擎
    // soloud = new SoLoud::Soloud();
    // soloud->init();

    s_Initialized = true;
    std::cout << "[Audio] Initialized (stub - integrate SoLoud for real audio)" << std::endl;
}

void AudioEngine::Shutdown()
{
    if (!s_Initialized) return;

    // TODO: soloud->deinit();
    s_Initialized = false;
}

void AudioEngine::Update()
{
    // TODO: 更新 3D 监听器位置
}

void AudioSourceComponent::Play()
{
    if (!AudioEngine::IsInitialized()) return;
    Playing = true;
    // TODO: 实际播放
}

void AudioSourceComponent::Stop()
{
    Playing = false;
}

void AudioSourceComponent::Pause()
{
    Playing = false;
}
