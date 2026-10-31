#pragma once
#include <windows.h>
#include <thread>
#include <atomic>

class NovaFetchMainWindow {
public:
    explicit NovaFetchMainWindow(HINSTANCE instance) : instance_(instance) {}
    bool create();
    HWND handle() const { return hwnd_; }
private:
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT handleMessage(UINT, WPARAM, LPARAM);
    void startDownload();
    HINSTANCE instance_;
    HWND hwnd_=nullptr, url_=nullptr, path_=nullptr, status_=nullptr;
    HWND downloadButton_=nullptr, cancelButton_=nullptr;
    std::thread worker_;
    std::atomic<bool> running_{false};
};
