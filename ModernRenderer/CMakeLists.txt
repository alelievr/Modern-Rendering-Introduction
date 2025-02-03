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
include(3rdparty/imgui)

add_subdirectory(3rdparty/FlyCube)
add_subdirectory(3rdparty/FlyCube/src/Modules/AppBox)
add_subdirectory(3rdparty/FlyCube/src/Modules/AppSettings)
add_subdirectory(3rdparty/FlyCube/src/Modules/AppLoop)
add_subdirectory(3rdparty/assimp)
add_subdirectory(3rdparty/meshoptimizer)

include_directories(
    ${project_root}/3rdparty/assimp/include
    ${project_root}/3rdparty/stb
    ${project_root}/3rdparty/meshoptimizer/src
)

link_libraries(
    assimp
    meshoptimizer
)

add_executable(ModernRenderer WIN32
    src/main.cpp 
    src/Camera.cpp
    src/RenderDoc.cpp
    src/MatrixUtils.cpp
    src/Renderer.cpp
    src/Scene.cpp
    src/ModelImporter.cpp
    src/Model.cpp
    src/Material.cpp
    src/Mesh.cpp
    src/Texture.cpp
    src/RenderUtils.cpp
    src/MeshPool.cpp
)

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
set_property(TARGET ModernRenderer PROPERTY CXX_STANDARD 20)

set_target_properties(FlyCube PROPERTIES FOLDER 3rdparty)
set_target_properties(assimp uninstall unit UpdateAssimpLibsDebugSymbolsAndDLLs zlibstatic PROPERTIES FOLDER 3rdparty)

file(GLOB SHADER_FILES "${project_root}/assets/shaders/*.hlsl")
target_sources(ModernRenderer PRIVATE ${SHADER_FILES})
source_group("Shaders" FILES ${SHADER_FILES})
set_property(SOURCE ${SHADER_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

file(GLOB INCLUDE_FILES "${project_root}/src/*.hpp" "${project_root}/src/*.h")
target_sources(ModernRenderer PRIVATE ${INCLUDE_FILES})
source_group("Includes" FILES ${INCLUDE_FILES})
# set_property(SOURCE ${INCLUDE_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

add_compile_definitions(MODERN_RENDERER_ASSETS_PATH="${project_root}/assets/")

install(TARGETS ModernRenderer)

# Define the path to the DLL file
set(DLL_PATH "${project_root}/renderdoc.dll")

# Specify the folder to copy the DLL to, based on the configuration (e.g., Debug/Release)
set(DLL_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

# Add a custom command to copy the DLL after building the target
add_custom_command(TARGET ModernRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DLL_PATH}"  # Source DLL file
    "${DLL_OUTPUT_DIR}"  # Destination folder (e.g., Debug/Release directory)
    COMMENT "Copying DLL to output directory (${CMAKE_CFG_INTDIR})"
)
