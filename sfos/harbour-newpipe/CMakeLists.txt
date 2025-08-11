cmake_minimum_required(VERSION 3.5)
project(harbour-newpipe CXX)

find_package(Qt5 COMPONENTS Core Network Qml Gui Quick LinguistTools REQUIRED)

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)
pkg_search_module(SILICA REQUIRED sailfishsilica)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/share/harbour-newpipe/lib")

link_directories(${CMAKE_SOURCE_DIR}/lib/)

add_executable(harbour-newpipe src/harbour-newpipe.cpp)
target_compile_definitions(harbour-newpipe PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
target_include_directories(harbour-newpipe PRIVATE
    $<BUILD_INTERFACE:
    ${SAILFISH_INCLUDE_DIRS}>
    ${SILICA_INCLUDE_DIRS}
    PRIVATE "${CMAKE_SOURCE_DIR}/include"
)
target_link_libraries(harbour-newpipe
    Qt5::Quick
    ${SAILFISH_LDFLAGS}
    ${SILICA_LIBRARIES}
    -l:appwrapper.so
)
target_sources(harbour-newpipe PRIVATE
    "src/extractor.cpp"
    "src/filtermodel.cpp"
    "src/invoke.cpp"
    "src/searchmodel.cpp"
    "src/searchitem.cpp"
    "src/searchitemstream.cpp"
    "src/searchitemplaylist.cpp"
    "src/searchitemchannel.cpp"
    "src/searchitemcomment.cpp"
    "src/mediainfo.cpp"
    "src/utils.cpp"
    "src/imageprovider.cpp"
    "src/commentmodel.cpp"
    "src/commentitem.cpp"
    "src/pageref.cpp"
    "src/lifetimecheck.cpp"
    "src/channelinfo.cpp"
    "src/channeltabinfo.cpp"
    "src/listlinkhandler.cpp"
    "src/linkhandlermodel.cpp"
    "src/channelmodel.cpp"
    "src/channeltablistmodel.cpp"
    "src/tabbarlookup.cpp"
    "src/tabbartabinfo.cpp"
    "src/playlistmodel.cpp"
)

# Update the translations
# See https://doc.qt.io/archives/qt-5.15/cmake-manual.html
# See https://doc.qt.io/archives/qt-5.15/qtlinguist-cmake-qt5-create-translation.html
FILE(GLOB TsFiles "translations/*.ts")
set_source_files_properties(${TsFiles} PROPERTIES OUTPUT_LOCATION "translations/")
set(Qt5_LRELEASE_EXECUTABLE Qt5::lrelease -idbased)
qt5_create_translation(TRANSLATION_QM_FILES
    "qml/"
    "src/"
    ${TsFiles})

add_custom_target(translations ALL DEPENDS ${TRANSLATION_QM_FILES})
add_dependencies(harbour-newpipe translations)

install(TARGETS harbour-newpipe
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/harbour-newpipe
)
install(DIRECTORY translations
    DESTINATION share/harbour-newpipe
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES harbour-newpipe.desktop
    DESTINATION share/applications
)
install(FILES icons/86x86/harbour-newpipe.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/harbour-newpipe.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/harbour-newpipe.png
    DESTINATION share/icons/hicolor/128x128/apps
)
install(FILES icons/172x172/harbour-newpipe.png
    DESTINATION share/icons/hicolor/172x172/apps
)
install(FILES lib/appwrapper.so
    DESTINATION share/harbour-newpipe/lib
)
install(DIRECTORY qml/images/
    DESTINATION share/harbour-newpipe/qml/images
    FILES_MATCHING PATTERN "z*/*.png"
)

# Get the other files reachable from the project tree in Qt Creator
FILE(GLOB ImageFiles "qml/images/*/*.png")
add_custom_target(distfiles
    SOURCES
        harbour-newpipe.desktop
        qml/harbour-newpipe.qml
        qml/cover/CoverPage.qml
        qml/pages/SearchPage.qml
        qml/pages/VideoPage.qml
        qml/pages/RepliesPage.qml
        qml/pages/ChannelPage.qml
        qml/pages/PlaylistPage.qml
        qml/components/VideoPlayer.qml
        qml/components/IconButtonDual.qml
        qml/components/CommentItem.qml
        qml/components/MediaDetails.qml
        qml/components/KeyValue.qml
        qml/components/StatItem.qml
        qml/components/SearchThumbnail.qml
        qml/components/SearchDelegate.qml
        qml/components/TabBar.qml
        qml/components/TabButton.qml
        qml/components/TabItem.qml
        qml/components/Tab.qml
        qml/components/TabView.qml
        qml/components/ChannelAbout.qml
        qml/components/AboutKeyValue.qml
        qml/components/PlaylistDetails.qml
        qml/components/ProcessIndicator.qml
        qml/icons/
        rpm/harbour-newpipe.changes.in
        rpm/harbour-newpipe.changes.run.in
        rpm/harbour-newpipe.spec
        rpm/harbour-newpipe.yaml
        ${TsFiles}
        ${ImageFiles})

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
    "${CMAKE_INSTALL_PREFIX}\n${CMAKE_BINARY_DIR}/harbour-newpipe:bin\n")

