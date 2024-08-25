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

# set_target_properties(ModernRenderer PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD ON)
set_target_properties(FlyCube PROPERTIES FOLDER 3rdparty)

add_compile_definitions(MODERN_RENDERER_ASSETS_PATH="${project_root}/assets/")

install(TARGETS ModernRenderer)
