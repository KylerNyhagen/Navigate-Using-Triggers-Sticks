cmake_minimum_required(VERSION 3.21)
project(GamepadCursorMode VERSION 0.1.0 LANGUAGES CXX)

option(ENABLE_SKYRIM_SE "Build for Skyrim 1.5.x" OFF)
option(ENABLE_SKYRIM_AE "Build for Skyrim 1.6.x" ON)
option(ENABLE_SKYRIM_VR "Build for Skyrim VR" OFF)

configure_file(src/Version.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h" @ONLY)

set(COMMONLIBSSE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibSSE")
if(NOT EXISTS "${COMMONLIBSSE_DIR}/CMakeLists.txt")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone --branch ng --depth 1 --recurse-submodules --shallow-submodules
            https://github.com/alandtse/CommonLibSSE-NG "${COMMONLIBSSE_DIR}"
        RESULT_VARIABLE COMMONLIB_CLONE_RESULT)
    if(NOT COMMONLIB_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Could not clone CommonLibSSE-NG")
    endif()
endif()

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${COMMONLIBSSE_DIR}" CommonLibSSE)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED
    src/Config.cpp
    src/Config.h
    src/CursorController.cpp
    src/CursorController.h
    src/PCH.h
    src/Plugin.cpp
    src/UI.cpp
    src/UI.h
    src/UIStyle.h
    src/Version.h.in
    extern/SKSEMenuFramework.h)

target_include_directories(${PROJECT_NAME} PRIVATE src extern "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /EHsc /utf-8 /Zc:preprocessor)
target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE nlohmann_json::nlohmann_json spdlog::spdlog Xinput9_1_0)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if(DEFINED MOD_OUTPUT_FOLDER)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MOD_OUTPUT_FOLDER}/SKSE/Plugins"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${PROJECT_NAME}>" "${MOD_OUTPUT_FOLDER}/SKSE/Plugins/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/package/SKSE/Plugins/GamepadCursorMode.json" "${MOD_OUTPUT_FOLDER}/SKSE/Plugins/")
endif()
