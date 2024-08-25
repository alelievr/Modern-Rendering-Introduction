add_executable(ModernRenderer WIN32 main.cpp)

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
