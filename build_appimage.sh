#!/usr/bin/bash
set -e # Abbruch bei Fehlern

# --- 1. Konfiguration ---
# Passe diesen Pfad an, falls dein Qt woanders liegt!
# WICHTIG: Es muss das 'qmake' von Qt6 sein.
export QMAKE=/usr/lib/qt6/bin/qmake 

# Falls du Qt über den Online-Installer installiert hast, sieht der Pfad eher so aus:
# export QMAKE=$HOME/Qt/6.9.0/gcc_64/bin/qmake

# Arbeitsverzeichnisse
BUILD_DIR=build_appimage
APP_DIR=AppDir

# Aufräumen
rm -rf $BUILD_DIR $APP_DIR *.AppImage

# --- 2. Tools herunterladen (linuxdeploy) ---
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    echo "Downloading linuxdeploy..."
    wget -nc https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    echo "Downloading linuxdeploy-plugin-qt..."
    wget -nc https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
fi

# --- 3. Bauen & Installieren (in AppDir) ---
echo "Building Application..."
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# WICHTIG: CMAKE_INSTALL_PREFIX=/usr setzen! 
# AppImages erwarten, dass die Binary in usr/bin liegt.
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)

# Installation in das AppDir (simuliert das Root-Dateisystem)
make install DESTDIR=../$APP_DIR

cd ..

# --- 4. Zusätzliche Ressourcen kopieren ---
# Wir müssen das Icon und die Desktop-Datei an die richtigen Stellen im AppDir legen
echo "Deploying Icon and Desktop file..."

mkdir -p $APP_DIR/usr/share/icons/hicolor/256x256/apps/
#cp resources/img/qt_desktop-gallery.png $APP_DIR/usr/share/icons/hicolor/256x256/apps/qt_desktop-gallery.png
cp desktop-gallery.png $APP_DIR/usr/share/icons/hicolor/256x256/apps/desktop-gallery.png


mkdir -p $APP_DIR/usr/share/applications/
cp desktop-gallery.desktop $APP_DIR/usr/share/applications/

# --- 5. AppImage erstellen ---
echo "Generating AppImage..."

# Umgebungsvariablen für das Qt-Plugin
# KORREKTUR: Wir fordern explizit Styles, IconEngines und PlatformThemes an
export EXTRA_QT_PLUGINS="webengine,styles,iconengines,platformthemes,platforms"

# linuxdeploy aufrufen
# --appdir: Wo liegt unser installiertes Programm?
# --plugin qt: Nutze das Qt-Plugin (findet autom. Abhängigkeiten, QML, Plugins etc.)
# --output appimage: Erstelle am Ende die Datei
./linuxdeploy-x86_64.AppImage \
    --appdir $APP_DIR \
    --executable $APP_DIR/usr/bin/desktop-gallery \
    --desktop-file $APP_DIR/usr/share/applications/desktop-gallery.desktop \
    --icon-file desktop-gallery.png \
    --plugin qt \
    --output appimage

echo "Fertig! Das AppImage liegt im aktuellen Ordner."