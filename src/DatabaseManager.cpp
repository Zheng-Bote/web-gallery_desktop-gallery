/**
 * @file DatabaseManager.cpp
 * @author ZHENG Robert
 * @brief Implementation of the DatabaseManager class.
 */
#include "DatabaseManager.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

DatabaseManager &DatabaseManager::instance() {
  static DatabaseManager _instance;
  return _instance;
}

bool DatabaseManager::init() {
  // If init() is called multiple times
  if (m_db.isOpen()) {
    return true;
  }

  m_db = QSqlDatabase::addDatabase("QSQLITE");

  // IMPORTANT: We use CacheLocation. This is the correct place for such data.
  // Path looks like e.g.:
  // Windows: C:/Users/<User>/AppData/Local/ZHENG
  // Robert/Desktop-Gallery/cache/gallery_cache.db Linux:
  // /home/<User>/.cache/Desktop-Gallery/gallery_cache.db
  QString cacheLocation =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

  // Create folder if it doesn't exist! (SQLite doesn't do this itself)
  QDir dir(cacheLocation);
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      qCritical() << "FATAL: Could not create cache directory:"
                  << cacheLocation;
      return false;
    }
  }

  m_dbPath = dir.filePath("gallery_cache.db");
  if (QFile::exists(m_dbPath)) {
    if (QFile::remove(m_dbPath)) {
      qDebug() << "Old cache database deleted.";
    } else {
      qWarning() << "Could not delete old cache database (maybe locked?):"
                 << m_dbPath;
    }
  }

  qDebug() << "Database Path:" << m_dbPath;

  m_db.setDatabaseName(m_dbPath);

  if (!m_db.open()) {
    qCritical() << "DB init failed:" << m_db.lastError().text();
    qCritical() << "Path was:" << m_dbPath;
    return false;
  }

  // Performance Optimization
  QSqlQuery query;
  query.exec("PRAGMA journal_mode = WAL;");
  query.exec("PRAGMA synchronous = NORMAL;");

  // Create Table
  bool success = query.exec(R"(
        CREATE TABLE IF NOT EXISTS images (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT UNIQUE,
            filename TEXT,
            folder_path TEXT,
            file_size INTEGER,
            
            -- Metadata Cache
            exif_datetime TEXT,
            exif_make TEXT,
            exif_model TEXT,
            
            -- For Overlay Display
            copyright TEXT,
            gps_decimal TEXT
        );
    )");

  if (!success) {
    qCritical() << "Could not create table 'images':"
                << query.lastError().text();
    return false;
  }

  query.exec("CREATE INDEX IF NOT EXISTS idx_folder ON images(folder_path);");

  return true;
}

QSqlDatabase &DatabaseManager::getDb() { return m_db; }

QString DatabaseManager::getDatabasePath() const { return m_dbPath; }

void DatabaseManager::clearDatabase() {
  if (m_db.isOpen()) {
    QSqlQuery query;
    query.exec("DELETE FROM images");
  }
}