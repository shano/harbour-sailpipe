find_package(Qt5 COMPONENTS Core Test QUIET)

set(CMAKE_AUTOMOC ON)

add_executable(test_ytdlptranslate
    testytdlptranslate.cpp
    "${PROJECT_SOURCE_DIR}/src/ytdlptranslate.cpp"
)
target_compile_definitions(test_ytdlptranslate PRIVATE
    TESTDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(test_ytdlptranslate
    Qt5::Core
    Qt5::Test
)
add_test(NAME ytdlptranslate COMMAND test_ytdlptranslate)
