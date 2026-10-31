add_library(novafetch_core
 core/download_manager.cpp core/download_task.cpp core/download_queue.cpp core/download_segment.cpp core/download_scheduler.cpp
 network/http_client.cpp network/url_parser.cpp network/http_headers.cpp
 storage/file_writer.cpp storage/metadata_store.cpp storage/settings.cpp
 system/portable_paths.cpp system/clipboard_monitor.cpp system/notifications.cpp
 ui/main_window.cpp ui/download_list.cpp ui/download_dialog.cpp ui/settings_window.cpp ui/theme.cpp ui/controls.cpp)
target_include_directories(novafetch_core PUBLIC ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(novafetch_core PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)
target_compile_definitions(novafetch_core PUBLIC UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
if(WIN32)
 target_link_libraries(novafetch_core PUBLIC comctl32 shell32)
endif()
