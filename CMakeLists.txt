cmake_minimum_required(VERSION 3.20)
project(DnD5E_CharGen VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Boost.JSON is built into dndcore via its header-only unity implementation.
# Boost headers are a build dependency only; no Boost runtime DLL is required.
find_package(Boost 1.75 REQUIRED)

add_library(dndcore STATIC
    src/json.cpp
    src/content_registry.cpp
    src/character.cpp
)
target_include_directories(dndcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_compile_definitions(dndcore PUBLIC BOOST_JSON_NO_LIB)

add_executable(dnd5e-cli apps/cli.cpp)
target_link_libraries(dnd5e-cli PRIVATE dndcore)

option(DND_BUILD_GUI "Build the Qt 6 desktop app for macOS, Windows and Linux" ON)
if(DND_BUILD_GUI)
    find_package(Qt6 6.4 COMPONENTS Widgets QUIET)
    if(Qt6_FOUND)
        qt_add_executable(dnd5e-gui MANUAL_FINALIZATION apps/gui/main.cpp)
        target_link_libraries(dnd5e-gui PRIVATE dndcore Qt6::Widgets)
        target_compile_definitions(dnd5e-gui PRIVATE DND_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
        set_target_properties(dnd5e-gui PROPERTIES
            MACOSX_BUNDLE TRUE
            WIN32_EXECUTABLE TRUE
        )
        qt_finalize_executable(dnd5e-gui)
        install(TARGETS dnd5e-gui BUNDLE DESTINATION . RUNTIME DESTINATION bin)
        if(COMMAND qt_generate_deploy_app_script)
            qt_generate_deploy_app_script(
                TARGET dnd5e-gui
                OUTPUT_SCRIPT deploy_script
                NO_UNSUPPORTED_PLATFORM_ERROR
            )
            install(SCRIPT ${deploy_script})
        endif()
    else()
        message(STATUS "Qt6 Widgets not found: building CLI and tests only. Install Qt 6.4+ for the desktop app.")
    endif()
endif()

install(DIRECTORY Content/ DESTINATION Content)

include(CTest)
if(BUILD_TESTING)
    add_executable(dnd5e-tests tests/test_core.cpp)
    target_link_libraries(dnd5e-tests PRIVATE dndcore)
    target_compile_definitions(dnd5e-tests PRIVATE DND_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME dnd5e-core COMMAND dnd5e-tests)
endif()
