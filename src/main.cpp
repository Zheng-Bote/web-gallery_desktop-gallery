/**
 * @file main.cpp
 * @author ZHENG Robert
 * @brief Entry point
 */
#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QTranslator>

#include "MainWindow.hpp"
#include "rz_config.hpp" // Falls du Version/Name dort definiert hast

// Fallback, falls rz_config.hpp fehlt
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
  QApplication a(argc, argv);

  // WICHTIG: Metadaten setzen, BEVOR irgendwelche Pfade (QStandardPaths)
  // abgerufen werden! DatabaseManager nutzt dies für den Cache-Pfad.
  QCoreApplication::setApplicationName(PROJECT_NAME);
  QCoreApplication::setApplicationVersion(PROJECT_VERSION);
  QCoreApplication::setOrganizationName(PROJECT_ORGANIZATION_NAME);
  // QCoreApplication::setOrganizationDomain("hase-zheng.net");

  // Icon setzen
  QApplication::setWindowIcon(
      QIcon(":/resources/img/qt_desktop-gallery_32x31.png"));

  MainWindow w;

  /* wir nutzen die vom Anwender gewählte Sprache
  // Sprache laden (Systemsprache erkennen)
  QString locale = QLocale::system().name(); // z.B. "de_DE"
  locale.truncate(locale.lastIndexOf('_'));  // z.B. "de"

  // Initiales Laden der Sprache
  w.loadLanguage(locale);
  */

  w.show();
  return a.exec();
}