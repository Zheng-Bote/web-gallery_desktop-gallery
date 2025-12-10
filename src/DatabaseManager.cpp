#include "DatabaseManager.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager _instance;
    return _instance;
}

bool DatabaseManager::init() {
    // Falls init() mehrfach aufgerufen wird
    if (m_db.isOpen()) {
        return true;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // WICHTIG: Wir nutzen CacheLocation. Das ist der korrekte Ort für solche Daten.
    // Pfad sieht z.B. so aus: 
    // Windows: C:/Users/<User>/AppData/Local/ZHENG Robert/Desktop-Gallery/cache/gallery_cache.db
    // Linux:   /home/<User>/.cache/Desktop-Gallery/gallery_cache.db
    QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    
    // Ordner erstellen, falls er nicht existiert! (SQLite macht das nicht selbst)
    QDir dir(cacheLocation);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qCritical() << "FATAL: Konnte Cache-Ordner nicht erstellen:" << cacheLocation;
            return false;
        }
    }

    m_dbPath = dir.filePath("gallery_cache.db");
    if (QFile::exists(m_dbPath)) {
        if (QFile::remove(m_dbPath)) {
            qDebug() << "Alte Cache-Datenbank gelöscht.";
        } else {
            qWarning() << "Konnte alte Cache-Datenbank nicht löschen (evtl. gesperrt?):" << m_dbPath;
        }
    }
    
    qDebug() << "Datenbank-Pfad:" << m_dbPath;

    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qCritical() << "DB init failed:" << m_db.lastError().text();
        qCritical() << "Pfad war:" << m_dbPath;
        return false;
    }

    // Performance-Optimierung
    QSqlQuery query;
    query.exec("PRAGMA journal_mode = WAL;");
    query.exec("PRAGMA synchronous = NORMAL;");

    // Tabelle erstellen
    bool success = query.exec(R"(
        CREATE TABLE IF NOT EXISTS images (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT UNIQUE,
            filename TEXT,
            folder_path TEXT,
            file_size INTEGER,
            
            -- Metadaten Cache
            exif_datetime TEXT,
            exif_make TEXT,
            exif_model TEXT,
            
            -- Für Overlay-Anzeige
            copyright TEXT,
            gps_decimal TEXT
        );
    )");
    
    if(!success) {
        qCritical() << "Konnte Tabelle 'images' nicht erstellen:" << query.lastError().text();
        return false;
    }

    query.exec("CREATE INDEX IF NOT EXISTS idx_folder ON images(folder_path);");

    return true;
}

QSqlDatabase& DatabaseManager::getDb() {
    return m_db;
}

QString DatabaseManager::getDatabasePath() const {
    return m_dbPath;
}

void DatabaseManager::clearDatabase() {
    if (m_db.isOpen()) {
        QSqlQuery query;
        query.exec("DELETE FROM images");
    }
}