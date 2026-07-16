#include "fps_counter.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace GameCore::Monitor {

FpsCounter::FpsCounter()
{
    LARGE_INTEGER freq, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    frequency_ = freq.QuadPart;
    lastTime_  = now.QuadPart;
    fpsTimer_  = now.QuadPart;
}

void FpsCounter::Tick()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    const long long delta = now.QuadPart - lastTime_;
    frameTimeMs_ = (static_cast<double>(delta)
                    / static_cast<double>(frequency_)) * 1000.0;
    lastTime_ = now.QuadPart;
    ++frameCount_;

    const long long elapsed = now.QuadPart - fpsTimer_;
    if (elapsed >= frequency_) {
        fps_ = static_cast<double>(frameCount_)
               / (static_cast<double>(elapsed)
                  / static_cast<double>(frequency_));
        frameCount_ = 0;
        fpsTimer_   = now.QuadPart;
    }
}

} // namespace GameCore::Monitor