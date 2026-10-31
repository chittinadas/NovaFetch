#pragma once
#include <atomic>
#include <string>
namespace novafetch { struct DownloadTask {std::atomic<bool> cancel{false};std::atomic<double> progress{0};std::string url,destination;}; }
