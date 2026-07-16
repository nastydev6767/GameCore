#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <any>

namespace GameCore::Core {

using EventHandler = std::function<void(const std::any&)>;

class EventBus {
public:
    static EventBus& Instance();

    int  Subscribe  (const std::string& eventName, EventHandler handler);
    void Unsubscribe(const std::string& eventName, int id);
    void Publish    (const std::string& eventName, const std::any& data = {});

private:
    EventBus()  = default;
    ~EventBus() = default;

    EventBus(const EventBus&)            = delete;
    EventBus& operator=(const EventBus&) = delete;

    struct Subscription {
        int          id;
        EventHandler handler;
    };

    std::unordered_map<std::string, std::vector<Subscription>> subscribers_;
    std::mutex mutex_;
    int        nextId_ { 0 };
};

} // namespace GameCore::Core