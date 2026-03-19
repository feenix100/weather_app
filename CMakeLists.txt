cmake_minimum_required(VERSION 3.20)
project(cppweatherapp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Dear ImGui is fetched directly so the project remains easy to clone and build.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

set(GLFW_TARGET glfw)
if (TARGET glfw3::glfw)
    set(GLFW_TARGET glfw3::glfw)
endif()

target_link_libraries(imgui_lib PUBLIC ${GLFW_TARGET})

add_executable(cppweatherapp
    src/main.cpp
    src/App.cpp
    src/AnimationRenderer.cpp
    src/DateUtils.cpp
    src/GeocodingService.cpp
    src/HistoryService.cpp
    src/HttpClient.cpp
    src/WeatherCodeMap.cpp
    src/WeatherService.cpp
)

target_include_directories(cppweatherapp PRIVATE src)

target_link_libraries(cppweatherapp PRIVATE
    imgui_lib
    OpenGL::GL
    ${GLFW_TARGET}
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

if (WIN32)
    target_link_libraries(cppweatherapp PRIVATE ws2_32)
endif()
