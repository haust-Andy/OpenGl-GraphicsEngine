cmake_minimum_required(VERSION 3.16.0)
project(OpenGlEngine VERSION 1.0.0 LANGUAGES C CXX)

# ===== 构建配置 =====
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===== 外部依赖路径 =====
set(IMGUI_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-master)
set(GLM_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/external/glm-master)
set(INCLUDE_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ===== GLFW 库 =====
set(GLFW_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
if(WIN32)
    set(PLATFORM_LIBS opengl32 gdi32)
    set(GLFW_LIB     ${GLFW_LIB_PATH}/libglfw3.a)
    set(ASSIMP_LIB   ${GLFW_LIB_PATH}/libassimpd.dll.a)
    set(ASSIMP_DLL   ${GLFW_LIB_PATH}/libassimp-6d.dll)
else()
    set(PLATFORM_LIBS GL dl pthread)
    find_package(glfw3 REQUIRED)
    set(GLFW_LIB glfw)
    find_package(assimp REQUIRED)
    set(ASSIMP_LIB assimp)
endif()

# ===== ImGui 源文件 =====
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ===== GLAD 源文件 =====
set(GLAD_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/glad/glad.c)

# ===== 引擎源文件 =====
file(GLOB_RECURSE ENGINE_SOURCES
    "engine/core/*.cpp"
    "engine/renderer/*.cpp"
    "engine/resource/*.cpp"
    "engine/scene/*.cpp"
    "engine/postprocess/*.cpp"
    "engine/editor/*.cpp"
    "engine/physics/*.cpp"
    "engine/particle/*.cpp"
    "engine/audio/*.cpp"
    "engine/ui/*.cpp"
)

# ===== 引擎头文件 =====
file(GLOB_RECURSE ENGINE_HEADERS
    "engine/core/*.h"
    "engine/renderer/*.h"
    "engine/resource/*.h"
    "engine/scene/*.h"
    "engine/postprocess/*.h"
    "engine/editor/*.h"
    "engine/physics/*.h"
    "engine/particle/*.h"
    "engine/audio/*.h"
    "engine/ui/*.h"
)

# ===== 引擎静态库 =====
add_library(OpenGlEngine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
    ${IMGUI_SOURCES}
    ${GLAD_SOURCE}
)

target_include_directories(OpenGlEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}          # 项目根目录
    ${CMAKE_CURRENT_SOURCE_DIR}/engine   # engine/ 目录
    ${INCLUDE_DIR}                       # glad, GLFW, stb
    ${GLM_DIR}                           # glm
    ${IMGUI_DIR}                         # imgui
    ${IMGUI_DIR}/backends                # imgui backends
)

target_compile_definitions(OpenGlEngine PUBLIC
    GLM_ENABLE_EXPERIMENTAL              # glm/gtx 扩展
)

target_link_libraries(OpenGlEngine PUBLIC
    ${GLFW_LIB}
    ${ASSIMP_LIB}
    ${PLATFORM_LIBS}
)

# ===== 资源目录 =====
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(SHADER_DIR   "${CMAKE_SOURCE_DIR}/shader")

# ===== Sandbox 可执行文件 =====
add_executable(Sandbox
    app/SandboxApp.cpp
)

target_link_libraries(Sandbox PRIVATE OpenGlEngine)

# ===== 测试可执行文件 =====
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(Tests ${TEST_SOURCES})

target_include_directories(Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}          # engine/ 根目录
    ${INCLUDE_DIR}                       # glad, GLFW, stb
    ${GLM_DIR}                           # glm
)

target_link_libraries(Tests PRIVATE
    OpenGlEngine
    ${GLFW_LIB}
    ${ASSIMP_LIB}
    ${PLATFORM_LIBS}
)

# 测试资源复制
add_custom_command(TARGET Tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESOURCE_DIR} $<TARGET_FILE_DIR:Tests>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SHADER_DIR}   $<TARGET_FILE_DIR:Tests>/shader
)

if(WIN32 AND ASSIMP_DLL)
    add_custom_command(TARGET Tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ASSIMP_DLL}" $<TARGET_FILE_DIR:Tests>/
    )
endif()

# ===== Sandbox 资源复制 =====
add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESOURCE_DIR} $<TARGET_FILE_DIR:Sandbox>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SHADER_DIR}   $<TARGET_FILE_DIR:Sandbox>/shader
)

if(WIN32 AND ASSIMP_DLL)
    add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ASSIMP_DLL}" $<TARGET_FILE_DIR:Sandbox>/
    )
endif()

# ===== 编译选项 =====
if(MSVC)
    target_compile_options(OpenGlEngine PRIVATE /W3)
else()
    target_compile_options(OpenGlEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()
