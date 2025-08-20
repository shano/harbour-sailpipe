project(harbour-videotest CXX)
cmake_minimum_required(VERSION 3.5)

find_package (Qt5 COMPONENTS Core Network Qml Gui Quick REQUIRED)

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)
pkg_search_module(SILICA REQUIRED sailfishsilica)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(harbour-videotest src/harbour-videotest.cpp)
target_compile_definitions(harbour-videotest PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
target_include_directories(harbour-videotest PRIVATE
    $<BUILD_INTERFACE:
    ${SAILFISH_INCLUDE_DIRS}>
    ${SILICA_INCLUDE_DIRS}
)
target_link_libraries(harbour-videotest
    Qt5::Quick
    ${SAILFISH_LDFLAGS}
    ${SILICA_LIBRARIES}
)
target_sources(harbour-videotest PRIVATE
    "src/videomanagement.cpp"
)

install(TARGETS harbour-videotest
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/harbour-videotest
)
install(DIRECTORY translations
    DESTINATION share/harbour-videotest
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES harbour-videotest.desktop
    DESTINATION share/applications
)
install(FILES icons/86x86/harbour-videotest.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/harbour-videotest.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/harbour-videotest.png
    DESTINATION share/icons/hicolor/128x128/apps
)
install(FILES icons/172x172/harbour-videotest.png
    DESTINATION share/icons/hicolor/172x172/apps
)

# Get the other files reachable from the project tree in Qt Creator
FILE(GLOB TsFiles "translations/*.ts")
add_custom_target(distfiles
    SOURCES
        harbour-videotest.desktop
        qml/harbour-videotest.qml
        qml/cover/CoverPage.qml
        qml/pages/FirstPage.qml
        qml/pages/SecondPage.qml
        rpm/harbour-videotest.changes.in
        rpm/harbour-videotest.changes.run.in
        rpm/harbour-videotest.spec
        rpm/harbour-videotest.yaml
        ${TsFiles})

# Tell Qt Creator where the application executable(s) would be located on the
# device.
#
# It is not necessary to list other deployables than executables (runtime
# targets) here. The deployment process of Sailfish OS projects is opaque to
# Qt Creator and the information contained in QtCreatorDeployment.txt is only
# used to locate the executable associated with the active run configuration
# on the device in order to run it.
#
# Search the Qt Creator Manual to learn about the QtCreatorDeployment.txt file
# format.
file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/harbour-videotest:bin
")

