cmake_minimum_required(VERSION 3.23)

project(
  desktop-gallery
  VERSION 1.0.0
  DESCRIPTION "Desktop-Gallery for Linux, MacOS and Windows. Part of the Crow Web-Gallery project."
  HOMEPAGE_URL "https://github.com/Zheng-Bote/web-gallery_desktop-gallery"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# --- Dependencies ---
# WICHTIG: LinguistTools für Übersetzungen, Sql für Datenbank, Concurrent für Threads
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Sql Concurrent LinguistTools Network WebEngineWidgets)
find_package(exiv2lib REQUIRED CONFIG NAMES exiv2)

# --- Configure ---
set(EXECUTABLE_NAME "${PROJECT_NAME}")
set(PROG_LONGNAME "Desktop-Gallery")
set(PROG_CREATED "2025")
set(PROG_AUTHOR "ZHENG Bote")
set(PROG_ORGANIZATION_NAME "ZHENG Robert")
set(PROG_ORGANIZATION_DOMAIN "net.hase-zheng")
if(Qt6_FOUND)
    set(CMAKE_QTV "${Qt6_VERSION}")
else()
    set(CMAKE_QTV "Unknown")
endif()
add_subdirectory(configure)

# Include Verzeichnis global setzen
include_directories(${CMAKE_SOURCE_DIR}/include)

# --- I18N / Übersetzungen ---
# Liste hier deine Sprachdateien auf (müssen im Ordner i18n/ existieren)
set(TS_FILES
    i18n/gallery_de.ts
    i18n/gallery_en.ts
)

# .ts zu .qm kompilieren (REGEL 1: Hier werden die Regeln erstellt)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/i18n")
qt6_add_translation(QM_FILES ${TS_FILES})

# --- Sources & Headers ---
set(PROJECT_SOURCES
    # Source Files
    src/main.cpp
    src/MainWindow.cpp
    src/DatabaseManager.cpp
    src/ImageIndexer.cpp
    src/ThumbnailDelegate.cpp
    src/rz_photo.cpp
    src/rz_hwinfo.cpp
    src/DefaultMetaWidget.cpp
    src/picture_widget.cpp
    src/UploadManager.cpp
    src/MapWindow.cpp

    # Header Files (Zwingend notwendig für MOC!)
    include/MainWindow.hpp
    include/DatabaseManager.hpp
    include/ImageIndexer.hpp
    include/ThumbnailDelegate.hpp
    include/rz_photo.hpp
    include/rz_hwinfo.h
    include/DefaultMetaWidget.hpp
    include/picture_widget.h
    include/rz_metadata.hpp
    include/rz_config.hpp
    include/LoginDialog.hpp
    include/UploadManager.hpp
    include/MapWindow.hpp

    # ui
    src/picture_widget.ui
    src/DefaultMetaWidget.ui
    src/MainWindow.ui

    # Ressources
    resources.qrc

    # Übersetzungsdateien (Binary)
    ${QM_FILES}
)

# Executable erstellen
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Bibliotheken linken
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Sql
    Qt6::Concurrent
    Qt6::Network
    Qt6::WebEngineWidgets
    Exiv2::exiv2lib
)

# --- INSTALLATION RULES ---

# Das Executable nach /usr/bin (im AppDir Kontext)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Der Ordner i18n nach /usr/bin/i18n
install(FILES ${QM_FILES} DESTINATION bin/i18n)

# Auch die Ressourcen (falls du sie nicht in qrc hast)
# install(FILES resources/img/qt_desktop-gallery.png DESTINATION share/icons/hicolor/256x256/apps)

################
message(STATUS "Build setup complete for ${PROJECT_NAME}")

