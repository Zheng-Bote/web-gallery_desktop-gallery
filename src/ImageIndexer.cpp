#include "ImageIndexer.hpp"
#include "DatabaseManager.hpp"
#include "rz_photo.hpp" // WICHTIG: Hat gefehlt!

#include <QDirIterator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QDir>

ImageIndexer::ImageIndexer(const QString& folderPath) 
    : m_folderPath(folderPath) {}

void ImageIndexer::run() {
    qDebug() << "[Indexer] Starte Thread für:" << m_folderPath;

    QString connectionName = QString::number((quint64)QThread::currentThread(), 16);

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(DatabaseManager::instance().getDatabasePath());

        if (!db.open()) {
            qCritical() << "[Indexer] CRITICAL: Kann DB nicht öffnen!" << db.lastError().text();
            return;
        }

        QSqlQuery pragma(db);
        pragma.exec("PRAGMA synchronous = OFF;"); 
        pragma.exec("PRAGMA journal_mode = MEMORY;");

        db.transaction();
        
        // Query erweitert um Metadaten-Spalten (9 Parameter)
        QSqlQuery query(db);
        query.prepare("INSERT OR IGNORE INTO images "
                      "(path, filename, folder_path, file_size, exif_datetime, exif_make, exif_model, copyright, gps_decimal) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

        // Filter: Case-Insensitive ist hier nicht möglich im Iterator-Konstruktor unter Linux,
        // daher filtern wir "alle Dateien" manuell in der Schleife (siehe unten) oder nehmen Standard-Extensions.
        // Da rz_photo.cpp jetzt eine robuste Filter-Methode hat, nutzen wir hier einen breiten Filter
        // oder verlassen uns darauf, dass wir später keine 'falschen' Dateien indizieren.
        // Für Performance im Indexer nutzen wir erst mal Standard-Filter:
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png" << "*.webp" << "*.tiff" << "*.bmp";
        filters << "*.JPG" << "*.JPEG" << "*.PNG" << "*.WEBP" << "*.TIFF" << "*.BMP";

        QDirIterator it(m_folderPath, 
            filters, 
            QDir::Files, 
            QDirIterator::Subdirectories);

        int count = 0;
        int errorCount = 0;

        while (it.hasNext()) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                qDebug() << "[Indexer] Abbruch angefordert.";
                break;
            }

            it.next();
            QFileInfo fi = it.fileInfo();

            // Pfad normalisieren
            QString absPath = QDir::cleanPath(fi.absoluteFilePath());
            QString folderPath = QDir::cleanPath(fi.absolutePath());

            // --- Metadaten lesen ---
            Photo p(absPath);
            QString copyright = p.getXmpCopyrightOwner(); 
            QString gps = p.getGpsDecimalString();       
            QString date = ""; // Datum lassen wir erst mal leer für Speed, oder: p.getDateTime()...

            // Bindings (Reihenfolge beachten!)
            query.addBindValue(absPath);        // 1. path
            query.addBindValue(fi.fileName());  // 2. filename
            query.addBindValue(folderPath);     // 3. folder_path
            query.addBindValue(fi.size());      // 4. file_size
            query.addBindValue(date);           // 5. exif_datetime
            query.addBindValue("");             // 6. exif_make
            query.addBindValue("");             // 7. exif_model
            query.addBindValue(copyright);      // 8. copyright
            query.addBindValue(gps);            // 9. gps_decimal
            
            if (!query.exec()) {
                errorCount++;
                if (errorCount == 1) {
                    qCritical() << "[Indexer] SQL INSERT ERROR:" << query.lastError().text();
                    // KORREKTUR: absPath statt absFilePath verwenden
                    qCritical() << "[Indexer] Bei Datei:" << absPath; 
                }
            }

            // Batch Update für GUI (nicht zu oft)
            if (++count % 500 == 0) {
                emit progress(count);
            }
        }
        
        if (!db.commit()) {
            qCritical() << "[Indexer] COMMIT FAILED:" << db.lastError().text();
        } else {
            qDebug() << "[Indexer] Commit erfolgreich. Anzahl Dateien:" << count;
        }

        db.close();
    }

    QSqlDatabase::removeDatabase(connectionName);
    emit finished();
}