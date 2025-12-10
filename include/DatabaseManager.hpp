#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString> // Wichtig

class DatabaseManager {
public:
    static DatabaseManager& instance();
    bool init();
    QSqlDatabase& getDb();
    
    // NEU: Getter für den Dateipfad
    QString getDatabasePath() const;
    
    void clearDatabase();

private:
    DatabaseManager() = default;
    ~DatabaseManager() = default;
    
    // Wir speichern den Pfad als Member
    QString m_dbPath;
    QSqlDatabase m_db;
};