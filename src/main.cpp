/**
 * SPDX-FileComment: Entry point of the Desktop-Gallery application
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file main.cpp
 * @brief Entry point of the Desktop-Gallery application.
 * @version 0.2.0
 * @date 2026-03-08
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QTranslator>

#include <QStyleFactory> // Optional, für Style

#include "MainWindow.hpp"
#include "qt_gh-update-checker.hpp"
#include "rz_config.hpp" // If version/name is defined there

#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QtConcurrent>

#include <string_view>

// rz_config.hpp contains PROJECT_NAME, PROJECT_VERSION, PROG_ORGANIZATION_NAME

int main(int argc, char *argv[]) {

  // FIX: Disable hardware acceleration for WebEngine to avoid crashes
  // (GBM is not supported with the current configuration).
  // This forces Chromium into software rendering mode.
#ifdef __linux__
  for (int i = 1; i < argc; ++i) {
    if (std::string_view(argv[i]) == "--disable-gpu") {
      qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu");
      break;
    }
  }
#endif
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
  QCoreApplication::setApplicationName(QString::fromStdString(PROG_LONGNAME));
  QCoreApplication::setApplicationVersion(QString::fromStdString(PROJECT_VERSION));
  QCoreApplication::setOrganizationName(QString::fromStdString(PROG_ORGANIZATION_NAME));
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

  QtConcurrent::run([]() {
    try {
      auto info = qtgh::check_github_update(
          QString::fromStdString(PROJECT_HOMEPAGE_URL),
          QString::fromStdString(PROJECT_VERSION));
      if (info.hasUpdate) {
        QMetaObject::invokeMethod(qApp, [info]() {
          QMessageBox msgBox;
          msgBox.setWindowTitle(
              QCoreApplication::translate("main", "Update Available"));
          msgBox.setText(QCoreApplication::translate(
              "main", "A new version of the application is available."));
          msgBox.setInformativeText(
              QString("Current version: %1\nAvailable version: %2")
                  .arg(QString::fromStdString(PROJECT_VERSION),
                       info.latestVersion));
          QPushButton *downloadButton = msgBox.addButton(
              QCoreApplication::translate("main", "Download Release"),
              QMessageBox::ActionRole);
          msgBox.addButton(QMessageBox::Ignore);

          msgBox.exec();

          if (msgBox.clickedButton() == downloadButton) {
            QDesktopServices::openUrl(
                QUrl(QString::fromStdString(PROJECT_HOMEPAGE_URL) +
                     "/releases/latest"));
          }
        });
      }
    } catch (const std::exception &e) {
      qWarning() << "Update check failed: " << e.what();
    }
  });

  w.show();
  return a.exec();
}
