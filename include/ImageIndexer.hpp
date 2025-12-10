#pragma once
#include <QObject>
#include <QRunnable>
#include <QString>

class ImageIndexer : public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit ImageIndexer(const QString& folderPath);
    void run() override;

signals:
    void progress(int count);
    void finished();

private:
    QString m_folderPath;
};