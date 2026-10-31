#pragma once
#include <atomic>
#include <functional>
#include <string>

namespace novafetch {
class DownloadManager {
public:
    using Progress = std::function<void(double, long long, long long)>;
    bool download(const std::string& url, const std::string& destination,
                  Progress progress, std::string& error);
    static void cancelActive();
private:
    static std::atomic<bool> cancelled_;
};
}
