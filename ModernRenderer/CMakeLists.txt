cmake_minimum_required(VERSION 3.15)
project(ModernRenderer)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_SKIP_INSTALL_RULES True)
set(EXCLUDE_FROM_DEFAULT_BUILD false)

include(CMakeDependentOption)
option(VULKAN_SUPPORT "Vulkan support" OFF)
cmake_dependent_option(DIRECTX_SUPPORT "DirectX 12 support" ON "WIN32" OFF)
cmake_dependent_option(METAL_SUPPORT "Metal support" ON "APPLE" OFF)
add_compile_definitions(DIRECTX_SUPPORT=1) # Force D3D12 support for indirect commands support
option(BUILD_SAMPLES "Build samples" OFF)
option(BUILD_TESTING "Build unit tests" OFF)

set(project_root "${CMAKE_CURRENT_SOURCE_DIR}")
# add_library(SponzaPbrAssets INTERFACE)

list(PREPEND CMAKE_MODULE_PATH "${project_root}/3rdparty/cmake")
include(cmake_settings)
include(compiler_settings)

add_subdirectory(3rdparty/FlyCube)
include(3rdparty/imgui)
# include(3rdparty/glfw)

add_subdirectory(3rdparty/FlyCube/src/Modules/AppBox)
add_subdirectory(3rdparty/FlyCube/src/Modules/AppSettings)
add_subdirectory(3rdparty/FlyCube/src/Modules/AppLoop)

add_executable(ModernRenderer WIN32 src/main.cpp)

if (WIN32)
    set_target_properties(ModernRenderer PROPERTIES
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

target_link_libraries(ModernRenderer
    AppBox
    FlyCube
    FlyCubeAssets
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(FlyCube PROPERTIES FOLDER 3rdparty)

file(GLOB SHADER_FILES "${project_root}/assets/shaders/*.hlsl")
target_sources(ModernRenderer PRIVATE ${SHADER_FILES})
source_group("Shaders" FILES ${SHADER_FILES})
set_property(SOURCE ${SHADER_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

add_compile_definitions(MODERN_RENDERER_ASSETS_PATH="${project_root}/assets/")

install(TARGETS ModernRenderer)
