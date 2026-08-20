cmake_minimum_required(VERSION 3.5)
project(harbour-sailreel CXX)

enable_testing()

find_package(Qt5 COMPONENTS Core Network Qml Gui Quick LinguistTools REQUIRED)

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)
pkg_search_module(SILICA REQUIRED sailfishsilica)
pkg_search_module(TRANSFERENGINE REQUIRED nemotransferengine-qt5)
pkg_search_module(AMBERMPRIS REQUIRED ambermpris)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_definitions(SAILPIPE_VERSION="0.5")

add_executable(harbour-sailreel src/harbour-sailreel.cpp)
target_compile_definitions(harbour-sailreel PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
target_include_directories(harbour-sailreel PRIVATE
    $<BUILD_INTERFACE:
    ${SAILFISH_INCLUDE_DIRS}>
    ${SILICA_INCLUDE_DIRS}
    ${TRANSFERENGINE_INCLUDE_DIRS}
    ${AMBERMPRIS_INCLUDE_DIRS}
)
target_link_libraries(harbour-sailreel
    Qt5::Quick
    Qt5::Network
    ${SAILFISH_LDFLAGS}
    ${SILICA_LIBRARIES}
    ${TRANSFERENGINE_LIBRARIES}
    ${AMBERMPRIS_LIBRARIES}
)
target_sources(harbour-sailreel PRIVATE
    "src/extractor.cpp"
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
    "src/downloadmanager.cpp"
    "src/dbusadapter.cpp"
    "src/mediajunction.cpp"
    "src/subscriptionmanager.cpp"
    "src/watchhistorymanager.cpp"
    "src/ytdlpmanager.cpp"
    "src/ytdlpprocess.cpp"
    "src/ytdlptranslate.cpp"
    "src/ytdlpbackend.cpp"
    "src/ytdlpdownloadcontext.cpp"
    "src/sponsorblockclient.cpp"
)

# Update the translations
# See https://doc.qt.io/archives/qt-5.15/cmake-manual.html
# See https://doc.qt.io/archives/qt-5.15/qtlinguist-cmake-qt5-create-translation.html
FILE(GLOB TsFiles "translations/*.ts")
set_source_files_properties(${TsFiles} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/translations")
set(Qt5_LRELEASE_EXECUTABLE Qt5::lrelease -idbased)
qt5_create_translation(TRANSLATION_QM_FILES
    "qml/"
    "src/"
    ${TsFiles})

add_custom_target(translations ALL DEPENDS ${TRANSLATION_QM_FILES})
add_dependencies(harbour-sailreel translations)

install(TARGETS harbour-sailreel
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/harbour-sailreel
)
install(DIRECTORY translations
    DESTINATION share/harbour-sailreel
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES harbour-sailreel.desktop
    DESTINATION share/applications
)
install(FILES harbour-sailreel.profile
    DESTINATION /etc/sailjail/permissions
)
install(FILES icons/86x86/harbour-sailreel.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/harbour-sailreel.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/harbour-sailreel.png
    DESTINATION share/icons/hicolor/128x128/apps
)
install(FILES icons/172x172/harbour-sailreel.png
    DESTINATION share/icons/hicolor/172x172/apps
)
install(DIRECTORY qml/images/
    DESTINATION share/harbour-sailreel/qml/images
    FILES_MATCHING PATTERN "z*/*.png"
)

find_package(Qt5 COMPONENTS Test QUIET)
if(Qt5Test_FOUND)
  add_subdirectory(tests)
endif()

# Get the other files reachable from the project tree in Qt Creator
FILE(GLOB ImageFiles "qml/images/*/*.png")
add_custom_target(distfiles
    SOURCES
        harbour-sailreel.desktop
        qml/harbour-sailreel.qml
        qml/cover/CoverPage.qml
        qml/cover/Progress.qml
        qml/pages/SearchPage.qml
        qml/pages/VideoPage.qml
        qml/pages/RepliesPage.qml
        qml/pages/ChannelPage.qml
        qml/pages/PlaylistPage.qml
        qml/pages/SubscriptionsPage.qml
        qml/pages/ManageSubscriptionsPage.qml
        qml/pages/FullscreenVideoPage.qml
        qml/pages/AboutPage.qml
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
        qml/components/ActionBar.qml
        qml/components/InfoRow.qml
        qml/icons/
        rpm/harbour-sailreel.changes.in
        rpm/harbour-sailreel.changes.run.in
        rpm/harbour-sailreel.spec
        rpm/harbour-sailreel.yaml
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
    "${CMAKE_INSTALL_PREFIX}\n${CMAKE_BINARY_DIR}/harbour-sailreel:bin\n")

