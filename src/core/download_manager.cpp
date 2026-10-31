#include "download_manager.h"
#include "../network/url_parser.h"

#include <curl/curl.h>
#include <cstdio>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <thread>
#include <system_error>
#include <fstream>
#include <climits>

namespace novafetch {
std::atomic<bool> DownloadManager::cancelled_{false};

namespace {
struct ProgressContext {
    DownloadManager::Progress* callback;
    std::atomic<bool>* cancelled;
};

size_t writeCallback(char* data, size_t size, size_t count, void* output) {
    return std::fwrite(data, size, count, static_cast<FILE*>(output));
}

int progressCallback(void* opaque, curl_off_t total, curl_off_t now,
                     curl_off_t, curl_off_t) {
    auto* context = static_cast<ProgressContext*>(opaque);
    if (context && context->cancelled && context->cancelled->load()) return 1;
    if (context && context->callback && *context->callback) {
        const double percent = total > 0
            ? 100.0 * static_cast<double>(now) / static_cast<double>(total) : 0.0;
        (*context->callback)(percent, static_cast<long long>(now),
                             static_cast<long long>(total));
    }
    return 0;
}

bool isTransient(CURLcode code) {
    switch (code) {
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        case CURLE_OPERATION_TIMEDOUT:
        case CURLE_RECV_ERROR:
        case CURLE_SEND_ERROR:
        case CURLE_GOT_NOTHING:
        case CURLE_PARTIAL_FILE:
            return true;
        default:
            return false;
    }
}

bool perform(CURL* curl, FILE* file, ProgressContext& context,
             long long resumeOffset, CURLcode& result, long& httpStatus) {
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &context);
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE,
                     static_cast<curl_off_t>(resumeOffset));
    result = curl_easy_perform(curl);
    httpStatus = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    return result == CURLE_OK && httpStatus >= 200 && httpStatus < 400;
}
}

bool DownloadManager::download(const std::string& url,
                               const std::string& destination,
                               Progress progress,
                               std::string& error) {
    cancelled_.store(false);
    error.clear();

    if (!UrlParser::isHttpUrl(url)) {
        error = "Only HTTP and HTTPS URLs are supported.";
        return false;
    }

    static const bool curlReady =
        curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
    if (!curlReady) {
        error = "libcurl initialization failed.";
        return false;
    }

    namespace fs = std::filesystem;
    const fs::path finalPath = fs::u8path(destination);
    const fs::path partPath = fs::u8path(destination + ".part");

    std::error_code fsError;
    if (finalPath.has_parent_path()) {
        fs::create_directories(finalPath.parent_path(), fsError);
        if (fsError) {
            error = "Cannot create destination directory: " + fsError.message();
            return false;
        }
    }

    long long offset = 0;
    if (fs::exists(partPath, fsError) && !fsError) {
        const auto size = fs::file_size(partPath, fsError);
        if (!fsError && size <= static_cast<std::uintmax_t>(LLONG_MAX))
            offset = static_cast<long long>(size);
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        error = "Could not create HTTP session.";
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "NovaFetch/0.2");
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    ProgressContext context{&progress, &cancelled_};
    CURLcode result = CURLE_OK;
    long status = 0;
    bool success = false;

    // Resume an existing .part file. If the server refuses ranges, restart cleanly.
    for (int attempt = 0; attempt < 4 && !cancelled_.load(); ++attempt) {
        const char* mode = (offset > 0) ? "ab" : "wb";
#ifdef _WIN32
        FILE* output = _wfopen(partPath.wstring().c_str(),
                               offset > 0 ? L"ab" : L"wb");
#else
        FILE* output = std::fopen(partPath.string().c_str(), mode);
#endif
        if (!output) {
            error = "Cannot open temporary download file.";
            break;
        }

        success = perform(curl, output, context, offset, result, status);
        const bool writeFailed = std::ferror(output) != 0;
        std::fclose(output);

        if (success && !writeFailed) break;
        if (cancelled_.load()) {
            error = "Download cancelled. Partial data was kept for a later resume.";
            break;
        }
        if (writeFailed) {
            error = "A disk write failed. Check available storage and permissions.";
            break;
        }

        // A server that ignores/rejects Range must not have its full response appended.
        if (offset > 0 && (result == CURLE_RANGE_ERROR ||
                           status == 200 || status == 416)) {
            std::ofstream reset(partPath, std::ios::binary | std::ios::trunc);
            reset.close();
            offset = 0;
            continue;
        }

        if (status >= 400) {
            error = "HTTP server returned status " + std::to_string(status) + ".";
            break;
        }
        if (!isTransient(result) || attempt == 3) {
            error = curl_easy_strerror(result);
            break;
        }

        // Retain partial bytes; the next attempt asks the server to continue at that offset.
        fsError.clear();
        const auto currentSize = fs::file_size(partPath, fsError);
        if (!fsError && currentSize <= static_cast<std::uintmax_t>(LLONG_MAX))
            offset = static_cast<long long>(currentSize);
        std::this_thread::sleep_for(std::chrono::milliseconds(500 * (attempt + 1)));
    }

    curl_easy_cleanup(curl);

    if (!success || cancelled_.load()) {
        if (error.empty()) error = cancelled_
            ? "Download cancelled. Partial data was kept."
            : "Download did not complete.";
        return false;
    }

    fsError.clear();
    fs::rename(partPath, finalPath, fsError);
    if (fsError) {
        // Windows rename does not replace an existing destination.
        error = "Download completed, but could not finalize the file: " + fsError.message();
        return false;
    }
    return true;
}

void DownloadManager::cancelActive() {
    cancelled_.store(true);
}
}
