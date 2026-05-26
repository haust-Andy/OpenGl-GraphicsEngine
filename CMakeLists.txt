cmake_minimum_required(VERSION 3.10.0)
project(LearnOpenGl VERSION 0.1.0 LANGUAGES C CXX)

# 设置构建类型为 Debug（包含调试信息）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-master) 
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp             # 如果有就保留
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)


add_executable(LearnOpenGl src/main.cpp src/glad/glad.c ${IMGUI_SOURCES})

target_include_directories(LearnOpenGl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm-master
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${IMGUI_DIR}                    # 为了 "imgui.h"
    ${IMGUI_DIR}/backends           # 为了 "imgui_impl_glfw.h" 等
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_link_libraries(LearnOpenGl PRIVATE 
    opengl32       # Windows自带的OpenGL库
    gdi32          # Windows GUI依赖库
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libglfw3.a   
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libassimpd.dll.a
)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shader")


file(COPY ${RESOURCE_DIR} DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${SHADER_DIR} DESTINATION ${CMAKE_BINARY_DIR})
file(COPY "${CMAKE_SOURCE_DIR}/lib/libassimp-6d.dll"
    DESTINATION ${CMAKE_BINARY_DIR})
