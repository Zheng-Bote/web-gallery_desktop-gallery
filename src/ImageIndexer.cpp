/**
 * SPDX-FileComment: Implementation of the ImageIndexer class
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file ImageIndexer.cpp
 * @brief Implementation of the ImageIndexer class.
 * @version 1.0.0
 * @date 2026-03-07
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#include "ImageIndexer.hpp"
#include "DatabaseManager.hpp"
#include "rz_photo.hpp" // IMPORTANT: Was missing!

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QPair>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>

ImageIndexer::ImageIndexer(const QString &folderPath)
    : m_folderPath(folderPath) {}

void ImageIndexer::run() {
  qDebug() << "[Indexer] Starting thread for:" << m_folderPath;

  QString connectionName =
      QString("indexer_%1").arg(reinterpret_cast<quintptr>(this), 0, 16);

  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(DatabaseManager::instance().getDatabasePath());

    if (!db.open()) {
      qCritical() << "[Indexer] CRITICAL: Cannot open DB!"
                  << db.lastError().text();
      return;
    }

    QSqlQuery pragma(db);
    pragma.exec("PRAGMA busy_timeout = 5000;");
    pragma.exec("PRAGMA synchronous = NORMAL;");

    // Load existing cached file timestamps/sizes to skip unchanged files
    QHash<QString, QPair<qint64, qint64>> cachedFiles;
    {
      QSqlQuery cacheQuery(db);
      if (cacheQuery.exec(
              "SELECT path, file_size, file_mtime FROM images")) {
        while (cacheQuery.next()) {
          cachedFiles.insert(cacheQuery.value(0).toString(),
                             qMakePair(cacheQuery.value(1).toLongLong(),
                                       cacheQuery.value(2).toLongLong()));
        }
      }
    }

    db.transaction();

    // Query extended with metadata and file_mtime columns (10 parameters)
    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO images "
                  "(path, filename, folder_path, file_size, exif_datetime, "
                  "exif_make, exif_model, copyright, gps_decimal, file_mtime) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

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
      if (m_abort.load(std::memory_order_relaxed) ||
          QThread::currentThread()->isInterruptionRequested()) {
        qDebug() << "[Indexer] Abort requested.";
        break;
      }

      it.next();
      QFileInfo fi = it.fileInfo();

      // Normalize path
      QString absPath = QDir::cleanPath(fi.absoluteFilePath());
      QString folderPath = QDir::cleanPath(fi.absolutePath());
      const qint64 fileSize = fi.size();
      const qint64 fileMtime = fi.lastModified().toSecsSinceEpoch();

      if (auto cachedIt = cachedFiles.constFind(absPath);
          cachedIt != cachedFiles.cend() && cachedIt->second > 0 &&
          cachedIt->first == fileSize && cachedIt->second == fileMtime) {
        if (++count % 500 == 0) {
          emit progress(count);
        }
        continue;
      }

      // --- Read Metadata (Single-Pass) ---
      Photo p(absPath);
      const auto summary = p.readSummary();
      const QString &copyright = summary.copyright;
      const QString &gps = summary.gpsDecimal;
      QString date = ""; // Leave date empty for speed for now, or:
                         // p.getDateTime()...

      // Bindings (Observe order!)
      query.addBindValue(absPath);       // 1. path
      query.addBindValue(fi.fileName()); // 2. filename
      query.addBindValue(folderPath);    // 3. folder_path
      query.addBindValue(fileSize);      // 4. file_size
      query.addBindValue(date);          // 5. exif_datetime
      query.addBindValue("");            // 6. exif_make
      query.addBindValue("");            // 7. exif_model
      query.addBindValue(copyright);     // 8. copyright
      query.addBindValue(gps);           // 9. gps_decimal
      query.addBindValue(fileMtime);     // 10. file_mtime

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