# NovaFetch

A lightweight Windows HTTP/HTTPS download-manager prototype built with C++20, Win32 and libcurl (no Electron).

## Implemented in this iteration
- Native Windows UI.
- HTTP and HTTPS downloads with redirect following.
- Progress reporting and a Cancel button.
- Partial download retention in `destination.part`.
- Resume attempts from an existing partial file when the server supports HTTP Range.
- Bounded retries for selected transient network errors.
- Connection timeout, low-speed timeout, HTTP error reporting and safer final-file promotion.
- CMake + vcpkg build configuration and GitHub Actions Windows workflows.

## Known limitations
This is not yet an IDM replacement. Multi-segment downloading, queue management, pause/resume while the process stays open, browser integration, clipboard URL detection, authentication/cookie import, checksum verification, and persistent settings are not implemented. Resume depends on server Range support and may restart when a server rejects range requests. Existing destination files are not automatically overwritten by the final rename.

No honest developer can certify “90% bug-free” without compiling on the target environment, running automated tests, and testing real servers and Windows versions. The included workflows are intended to perform the Windows build and test.

## Build on Windows
Install Visual Studio 2022 with Desktop C++, CMake 3.24+, Git and vcpkg. Set `VCPKG_ROOT`, then:
```powershell
cmake --preset windows-release
cmake --build --preset windows-release
ctest --test-dir build -C Release --output-on-failure
```
Use the GitHub Actions `Windows Build` workflow to build on a Windows runner from Codespaces.
