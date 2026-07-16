#include "event_bus.h"
#include <algorithm>

namespace GameCore::Core {

EventBus& EventBus::Instance()
{
    static EventBus instance;
    return instance;
}

int EventBus::Subscribe(const std::string& eventName, EventHandler handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    const int id = nextId_++;
    subscribers_[eventName].push_back({ id, std::move(handler) });
    return id;
}

void EventBus::Unsubscribe(const std::string& eventName, int id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(eventName);
    if (it == subscribers_.end()) return;

    auto& subs = it->second;
    subs.erase(
        std::remove_if(subs.begin(), subs.end(),
            [id](const Subscription& s) { return s.id == id; }),
        subs.end());
}

void EventBus::Publish(const std::string& eventName, const std::any& data)
{
    std::vector<Subscription> snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers_.find(eventName);
        if (it == subscribers_.end()) return;
        snapshot = it->second;
    }
    for (const auto& sub : snapshot)
        sub.handler(data);
}

} // namespace GameCore::Core