add_executable(NovaFetch WIN32 main.cpp application.cpp ../resources/app.rc)
target_link_libraries(NovaFetch PRIVATE novafetch_core)
