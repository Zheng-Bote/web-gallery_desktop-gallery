/**
 * @file ImageIndexer.cpp
 * @author ZHENG Robert
 * @brief Implementation of the ImageIndexer class.
 */
#include "ImageIndexer.hpp"
#include "DatabaseManager.hpp"
#include "rz_photo.hpp" // IMPORTANT: Was missing!

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>

ImageIndexer::ImageIndexer(const QString &folderPath)
    : m_folderPath(folderPath) {}

void ImageIndexer::run() {
  qDebug() << "[Indexer] Starting thread for:" << m_folderPath;

  QString connectionName =
      QString::number((quint64)QThread::currentThread(), 16);

  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(DatabaseManager::instance().getDatabasePath());

    if (!db.open()) {
      qCritical() << "[Indexer] CRITICAL: Cannot open DB!"
                  << db.lastError().text();
      return;
    }

    QSqlQuery pragma(db);
    pragma.exec("PRAGMA synchronous = OFF;");
    pragma.exec("PRAGMA journal_mode = MEMORY;");

    db.transaction();

    // Query extended with metadata columns (9 parameters)
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO images "
                  "(path, filename, folder_path, file_size, exif_datetime, "
                  "exif_make, exif_model, copyright, gps_decimal) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

    // Filter: Case-Insensitive is not possible in the Iterator constructor here
    // under Linux, so we filter "all files" manually in the loop
    // (see below) or use standard extensions. Since rz_photo.cpp now has a
    // robust filter method, we use a broad filter here or rely on the fact that
    // we don't index 'wrong' files later. For performance in the Indexer we use
    // standard filters for now:
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.webp" << "*.tiff"
            << "*.bmp";
    filters << "*.JPG" << "*.JPEG" << "*.PNG" << "*.WEBP" << "*.TIFF"
            << "*.BMP";

    QDirIterator it(m_folderPath, filters, QDir::Files,
                    QDirIterator::Subdirectories);

    int count = 0;
    int errorCount = 0;

    while (it.hasNext()) {
      if (QThread::currentThread()->isInterruptionRequested()) {
        qDebug() << "[Indexer] Abort requested.";
        break;
      }

      it.next();
      QFileInfo fi = it.fileInfo();

      // Normalize path
      QString absPath = QDir::cleanPath(fi.absoluteFilePath());
      QString folderPath = QDir::cleanPath(fi.absolutePath());

      // --- Read Metadata ---
      Photo p(absPath);
      QString copyright = p.getCopyright();
      QString gps = p.getGpsDecimalString();
      QString date = ""; // Leave date empty for speed for now, or:
                         // p.getDateTime()...

      // Bindings (Observe order!)
      query.addBindValue(absPath);       // 1. path
      query.addBindValue(fi.fileName()); // 2. filename
      query.addBindValue(folderPath);    // 3. folder_path
      query.addBindValue(fi.size());     // 4. file_size
      query.addBindValue(date);          // 5. exif_datetime
      query.addBindValue("");            // 6. exif_make
      query.addBindValue("");            // 7. exif_model
      query.addBindValue(copyright);     // 8. copyright
      query.addBindValue(gps);           // 9. gps_decimal

      if (!query.exec()) {
        errorCount++;
        if (errorCount == 1) {
          qCritical() << "[Indexer] SQL INSERT ERROR:"
                      << query.lastError().text();
          // CORRECTION: Use absPath instead of absFilePath
          qCritical() << "[Indexer] At file:" << absPath;
        }
      }

      // Batch Update for GUI (not too often)
      if (++count % 500 == 0) {
        emit progress(count);
      }
    }

    if (!db.commit()) {
      qCritical() << "[Indexer] COMMIT FAILED:" << db.lastError().text();
    } else {
      qDebug() << "[Indexer] Commit successful. Number of files:" << count;
    }

    db.close();
  }

  QSqlDatabase::removeDatabase(connectionName);
  emit finished();
}