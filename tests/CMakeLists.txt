find_package(Catch2 3 REQUIRED)

add_executable(ELMTests
    test_language_manager.cpp
    test_ini_parser.cpp
)

target_include_directories(ELMTests PRIVATE ${CMAKE_SOURCE_DIR})

if(wxWidgets_LIBRARIES)
    target_link_libraries(ELMTests PRIVATE Catch2::Catch2WithMain ${wxWidgets_LIBRARIES})
else()
    target_link_libraries(ELMTests PRIVATE Catch2::Catch2WithMain wx::core wx::base)
endif()

target_sources(ELMTests PRIVATE
    ${CMAKE_SOURCE_DIR}/LanguageManager.cpp
    ${CMAKE_SOURCE_DIR}/IniParser.cpp
)

include(Catch)
catch_discover_tests(ELMTests)
