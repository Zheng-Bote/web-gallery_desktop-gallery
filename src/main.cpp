/**
 * @file main.cpp
 * @author ZHENG Robert
 * @brief Entry point of the Desktop-Gallery application.
 */
#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QTranslator>

#include <QStyleFactory> // Optional, für Style

#include "MainWindow.hpp"
#include "rz_config.hpp" // If version/name is defined there

// Fallback if rz_config.hpp is missing
#ifndef PROJECT_NAME
#define PROJECT_NAME "Desktop-Gallery"
#endif
#ifndef PROJECT_VERSION
#define PROJECT_VERSION "0.24.0"
#endif
#ifndef PROJECT_ORGANIZATION_NAME
#define PROJECT_ORGANIZATION_NAME "ZHENG Robert"
#endif

int main(int argc, char *argv[]) {

  // FIX: Disable hardware acceleration for WebEngine to avoid crashes
  // (GBM is not supported with the current configuration).
  // This forces Chromium into software rendering mode.
  qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu");
  // if it still crashes with your graphic card, here the hardcore version:
  // qputenv("QT_XCB_GL_INTEGRATION", "none");

  // 2. Umgebungsvariablen säubern (gegen Theme-Probleme)
  // qputenv("QT_QPA_PLATFORMTHEME", "xdgdesktopportal");
  // qputenv("UBUNTU_MENUPROXY", "0");

  // Fallback auf xcb ist gut, lassen wir drin
  // qputenv("QT_QPA_PLATFORM", "xcb");

  // 3. WICHTIG: Natives Menü global verbieten!
  // Muss VOR dem QApplication Konstruktor passieren.
  // QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

  QApplication a(argc, argv);

  // IMPORTANT: Set metadata BEFORE accessing any paths (QStandardPaths)!
  // DatabaseManager uses this for the cache path.
  QCoreApplication::setApplicationName(PROJECT_NAME);
  QCoreApplication::setApplicationVersion(PROJECT_VERSION);
  QCoreApplication::setOrganizationName(PROJECT_ORGANIZATION_NAME);
  // QCoreApplication::setOrganizationDomain("hase-zheng.net");

  // Set Icon
  QApplication::setWindowIcon(
      QIcon(":/resources/img/qt_desktop-gallery_32x31.png"));

  MainWindow w;

  /* We use the language selected by the user
  // Sprache laden (Systemsprache erkennen)
  QString locale = QLocale::system().name(); // z.B. "de_DE"
  locale.truncate(locale.lastIndexOf('_'));  // z.B. "de"

  // Initial load of the language
  w.loadLanguage(locale);
  */

  w.show();
  return a.exec();
}