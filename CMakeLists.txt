cmake_minimum_required(VERSION 3.15)
project(EasyLoginManager)

# Ensure MSVC runtime library selection via CMAKE_MSVC_RUNTIME_LIBRARY
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force static CRT for MSVC on Windows when using static triplet
if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "static")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Find wxWidgets
if(VCPKG_TARGET_TRIPLET MATCHES "static")
    set(wxWidgets_USE_STATIC ON)
endif()

# Try CONFIG mode first (best for vcpkg/manifest)
find_package(wxWidgets CONFIG)
if(NOT wxWidgets_FOUND)
    find_package(wxWidgets REQUIRED COMPONENTS core base)
endif()

if(wxWidgets_FOUND AND wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
endif()

# Add executable
if(WIN32)
    set(EXE_RESOURCES resources.rc)
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set(EXE_RESOURCES icon.icns)
    set_source_files_properties(${EXE_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(EasyLoginManager MACOSX_BUNDLE
        main.cpp
        AccountDialog.cpp
        MainFrame.cpp
        SettingsDialog.cpp
        LanguageManager.cpp
        IniParser.cpp
        ${EXE_RESOURCES}
)

# i18n
set(I18N_FILES 
        i18n/en.json 
        i18n/fr.json 
        i18n/el.json 
        i18n/pt.json 
        i18n/pl.json 
        i18n/os.json 
        i18n/zh.json
)
if(APPLE)
    set_source_files_properties(${I18N_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/i18n")
    target_sources(EasyLoginManager PRIVATE ${I18N_FILES})
else()
    # On Windows/Linux, we might want to copy them to the build directory
    foreach(file ${I18N_FILES})
        get_filename_component(filename ${file} NAME)
        add_custom_command(TARGET EasyLoginManager POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:EasyLoginManager>/i18n
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${file} $<TARGET_FILE_DIR:EasyLoginManager>/i18n/${filename}
        )
    endforeach()
endif()

# Link wxWidgets
if(wxWidgets_LIBRARIES)
    target_link_libraries(EasyLoginManager ${wxWidgets_LIBRARIES})
else()
    target_link_libraries(EasyLoginManager wx::core wx::base)
endif()

# macOS specific: set bundle properties
if(APPLE)
    set_target_properties(EasyLoginManager PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.l2reborn.easyloginmanager"
            MACOSX_BUNDLE_BUNDLE_NAME "EasyLoginManager"
            MACOSX_BUNDLE_INFO_PLIST_VERSION "6.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    )
endif()

# Windows specific: remove console window and set UTF-8 encoding
if(WIN32)
    set_target_properties(EasyLoginManager PROPERTIES WIN32_EXECUTABLE TRUE)
    target_compile_options(EasyLoginManager PRIVATE /utf-8)
endif()

# Unit tests (Catch2). Opt-in so the default app build stays dependency-light.
option(ELM_BUILD_TESTS "Build EasyLoginManager unit tests" OFF)
if(ELM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
