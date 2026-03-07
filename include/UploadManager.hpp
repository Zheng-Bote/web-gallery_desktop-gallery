/**
 * SPDX-FileComment: Manages image uploads to a remote server
 * SPDX-FileType: HEADER
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file UploadManager.hpp
 * @brief Manages image uploads to a remote server.
 * @version 1.0.0
 * @date 2026-03-07
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QString>

/**
 * @class UploadManager
 * @brief Manages image uploads to a remote server.
 *
 * Handles authentication (login, token refresh), queueing of files,
 * and the actual upload process via HTTP POST requests.
 */
class UploadManager : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent The parent object.
   */
  explicit UploadManager(QObject *parent = nullptr);

  /**
   * @brief Set the server URL.
   * @param url The base URL of the remote server.
   */
  void setServerUrl(const QString &url);

  /**
   * @brief Authenticate with the server.
   * @param user Username.
   * @param pass Password.
   */
  void login(const QString &user, const QString &pass);

  /**
   * @brief Start uploading files.
   * @param filePaths List of local file paths to upload.
   * @param serverPath The target directory path on the server.
   */
  // CORRECTION: Added serverPath parameter
  void uploadFiles(const QList<QString> &filePaths, const QString &serverPath);

signals:
  /** @brief Emitted when login is successful. */
  void loginSuccessful();

  /** @brief Emitted when login fails. */
  void loginFailed(const QString &message);

  /**
   * @brief Emitted to report upload progress.
   * @param currentFileIdx Index of the file currently being uploaded.
   * @param totalFiles Total number of files in the queue.
   * @param percent Progress of the current file in percent.
   */
  void uploadProgress(int currentFileIdx, int totalFiles, int percent);

  /** @brief Emitted when a single file upload finishes. */
  void fileUploaded(const QString &filename, bool success);

  /** @brief Emitted when all uploads are finished. */
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

  // NEW: Target path on the server
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