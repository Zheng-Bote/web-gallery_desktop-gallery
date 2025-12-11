#pragma once

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QString>

class UploadManager : public QObject {
  Q_OBJECT

public:
  explicit UploadManager(QObject *parent = nullptr);

  void setServerUrl(const QString &url);
  void login(const QString &user, const QString &pass);

  // KORREKTUR: serverPath Parameter hinzugefügt
  void uploadFiles(const QList<QString> &filePaths, const QString &serverPath);

signals:
  void loginSuccessful();
  void loginFailed(const QString &message);
  void uploadProgress(int currentFileIdx, int totalFiles, int percent);
  void fileUploaded(const QString &filename, bool success);
  void allFinished();

private slots:
  void onNetworkFinished(QNetworkReply *reply);
  void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
  void startNextUpload();
  void performTokenRefresh();
  void retryLastAction();

  QNetworkAccessManager *m_netManager;
  QString m_serverUrl;

  QString m_jwtToken;
  QString m_refreshToken;
  bool m_isRefreshing{false};

  // NEU: Zielpfad auf dem Server
  QString m_targetServerPath;

  struct UploadJob {
    QString filePath;
  };
  QQueue<UploadJob> m_queue;
  int m_totalFiles{0};
  int m_processedFiles{0};

  enum State { IDLE, LOGGING_IN, UPLOADING };
  State m_state{IDLE};

  QString m_lastUser;
  QString m_lastPass;
};