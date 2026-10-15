
cmake_minimum_required(VERSION 3.20)
project(SoulsborneDeathTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(json)

add_executable(DeathTrackerGUI WIN32 gui.cpp)

target_link_libraries(
    DeathTrackerGUI
    PRIVATE
    Qt6::Widgets
    Qt6::Network
    nlohmann_json::nlohmann_json
)