#pragma once

namespace GameCore::Monitor {

class FpsCounter {
public:
    FpsCounter();
    void Tick();

    double GetFps()         const { return fps_; }
    double GetFrameTimeMs() const { return frameTimeMs_; }

private:
    long long frequency_   { 0 };
    long long lastTime_    { 0 };
    long long fpsTimer_    { 0 };
    int       frameCount_  { 0 };
    double    fps_         { 0.0 };
    double    frameTimeMs_ { 0.0 };
};

} // namespace GameCore::Monitor