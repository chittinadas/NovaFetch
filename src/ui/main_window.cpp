#include "main_window.h"
#include "../core/download_manager.h"
#include "../network/url_parser.h"
#include <string>

namespace {
constexpr int ID_URL=101, ID_PATH=102, ID_DOWNLOAD=103, ID_CANCEL=104;
constexpr UINT WM_DOWNLOAD_DONE=WM_APP+1, WM_DOWNLOAD_PROGRESS=WM_APP+2;
struct ProgressMessage { double percent; };
}

bool NovaFetchMainWindow::create() {
    WNDCLASSW wc{};
    wc.lpfnWndProc=wndProc;
    wc.hInstance=instance_;
    wc.lpszClassName=L"NovaFetchWindow";
    wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    RegisterClassW(&wc);
    hwnd_=CreateWindowExW(0,wc.lpszClassName,L"NovaFetch",
        WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,700,285,
        nullptr,nullptr,instance_,this);
    return hwnd_!=nullptr;
}

LRESULT CALLBACK NovaFetchMainWindow::wndProc(HWND h,UINT m,WPARAM w,LPARAM l) {
    auto* self=reinterpret_cast<NovaFetchMainWindow*>(
        GetWindowLongPtrW(h,GWLP_USERDATA));
    if(m==WM_NCCREATE) {
        auto* cs=reinterpret_cast<CREATESTRUCTW*>(l);
        self=static_cast<NovaFetchMainWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));
    }
    return self ? self->handleMessage(m,w,l) : DefWindowProcW(h,m,w,l);
}

LRESULT NovaFetchMainWindow::handleMessage(UINT m,WPARAM w,LPARAM l) {
    switch(m) {
    case WM_CREATE:
        CreateWindowW(L"STATIC",L"HTTP/HTTPS URL",WS_CHILD|WS_VISIBLE,
            20,20,150,22,hwnd_,nullptr,instance_,nullptr);
        url_=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",
            WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,20,45,640,28,
            hwnd_,(HMENU)ID_URL,instance_,nullptr);
        CreateWindowW(L"STATIC",L"Destination (full file path)",WS_CHILD|WS_VISIBLE,
            20,85,220,22,hwnd_,nullptr,instance_,nullptr);
        path_=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"C:\\Downloads\\file.bin",
            WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,20,110,640,28,
            hwnd_,(HMENU)ID_PATH,instance_,nullptr);
        downloadButton_=CreateWindowW(L"BUTTON",L"Download / Resume",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,20,155,145,34,
            hwnd_,(HMENU)ID_DOWNLOAD,instance_,nullptr);
        cancelButton_=CreateWindowW(L"BUTTON",L"Cancel",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,175,155,90,34,
            hwnd_,(HMENU)ID_CANCEL,instance_,nullptr);
        status_=CreateWindowW(L"STATIC",L"Ready. Interrupted downloads are kept as .part files.",
            WS_CHILD|WS_VISIBLE,20,205,640,28,hwnd_,nullptr,instance_,nullptr);
        EnableWindow(cancelButton_,FALSE);
        return 0;
    case WM_COMMAND:
        if(LOWORD(w)==ID_DOWNLOAD) { startDownload(); return 0; }
        if(LOWORD(w)==ID_CANCEL) {
            novafetch::DownloadManager::cancelActive();
            SetWindowTextW(status_,L"Cancelling...");
            return 0;
        }
        break;
    case WM_DOWNLOAD_PROGRESS: {
        auto* p=reinterpret_cast<ProgressMessage*>(l);
        if(p) {
            wchar_t text[100]{};
            swprintf_s(text,L"Downloading: %.1f%%",p->percent);
            SetWindowTextW(status_,text);
            delete p;
        }
        return 0;
    }
    case WM_DOWNLOAD_DONE: {
        auto* message=reinterpret_cast<std::wstring*>(l);
        if(message) {
            SetWindowTextW(status_,message->c_str());
            delete message;
        }
        running_=false;
        EnableWindow(downloadButton_,TRUE);
        EnableWindow(cancelButton_,FALSE);
        return 0;
    }
    case WM_DESTROY:
        novafetch::DownloadManager::cancelActive();
        if(worker_.joinable()) worker_.join();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd_,m,w,l);
}

void NovaFetchMainWindow::startDownload() {
    if(running_) return;
    wchar_t urlBuffer[4096]{}, pathBuffer[4096]{};
    GetWindowTextW(url_,urlBuffer,4096);
    GetWindowTextW(path_,pathBuffer,4096);
    const std::wstring wideUrl(urlBuffer), widePath(pathBuffer);
    const std::string url(wideUrl.begin(),wideUrl.end());
    const std::string path(widePath.begin(),widePath.end());
    if(!novafetch::isHttpUrl(url) || path.empty()) {
        SetWindowTextW(status_,L"Enter a valid HTTP/HTTPS URL and destination path.");
        return;
    }
    if(worker_.joinable()) worker_.join();
    running_=true;
    EnableWindow(downloadButton_,FALSE);
    EnableWindow(cancelButton_,TRUE);
    SetWindowTextW(status_,L"Starting download...");
    worker_=std::thread([this,url,path] {
        std::string error;
        const bool ok=novafetch::DownloadManager{}.download(
            url,path,[this](double percent,long long,long long) {
                auto* p=new ProgressMessage{percent};
                if(!PostMessageW(hwnd_,WM_DOWNLOAD_PROGRESS,0,
                    reinterpret_cast<LPARAM>(p))) delete p;
            },error);
        std::wstring result;
        if(ok) result=L"Download complete.";
        else {
            const std::string text=error.empty()?"Download failed.":error;
            result.assign(text.begin(),text.end());
        }
        auto* message=new std::wstring(std::move(result));
        if(!PostMessageW(hwnd_,WM_DOWNLOAD_DONE,0,
            reinterpret_cast<LPARAM>(message))) delete message;
    });
}
