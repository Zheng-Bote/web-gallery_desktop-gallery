/**
 * @file UploadManager.cpp
 * @author ZHENG Robert
 * @brief Implementation of the UploadManager class.
 */
#include "UploadManager.hpp"
#include "rz_photo.hpp"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>

UploadManager::UploadManager(QObject *parent) : QObject(parent) {
  m_netManager = new QNetworkAccessManager(this);
  connect(m_netManager, &QNetworkAccessManager::finished, this,
          &UploadManager::onNetworkFinished);
}

void UploadManager::setServerUrl(const QString &url) {
  m_serverUrl = url;
  if (m_serverUrl.endsWith("/"))
    m_serverUrl.chop(1);
}

void UploadManager::login(const QString &user, const QString &pass) {
  m_lastUser = user;
  m_lastPass = pass;
  m_state = LOGGING_IN;

  QUrl url(m_serverUrl + "/login");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = user;
  json["password"] = pass;

  m_netManager->post(request, QJsonDocument(json).toJson());
}

// CORRECTION: Store path
void UploadManager::uploadFiles(const QList<QString> &filePaths,
                                const QString &serverPath) {
  if (m_jwtToken.isEmpty()) {
    emit loginFailed("No active session. Please login first.");
    return;
  }

  m_targetServerPath = serverPath; // Save for later

  m_queue.clear();
  for (const QString &path : filePaths) {
    m_queue.enqueue({path});
  }
  m_totalFiles = m_queue.size();
  m_processedFiles = 0;

  m_state = UPLOADING;
  startNextUpload();
}

void UploadManager::startNextUpload() {
  if (m_queue.isEmpty()) {
    m_state = IDLE;
    emit allFinished();
    return;
  }

  UploadJob job = m_queue.head();

  QFile *file = new QFile(job.filePath);
  if (!file->open(QIODevice::ReadOnly)) {
    emit fileUploaded(QFileInfo(job.filePath).fileName(), false);
    m_queue.dequeue();
    m_processedFiles++;
    delete file;
    startNextUpload();
    return;
  }

  QUrl url(m_serverUrl + "/upload");
  QNetworkRequest request(url);
  QString bearer = "Bearer " + m_jwtToken;
  request.setRawHeader("Authorization", bearer.toUtf8());
  request.setRawHeader("User-Agent", "Desktop-Gallery-Client/1.0");

  QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  // 1. Image Part
  QHttpPart imagePart;
  QString fileName = QFileInfo(job.filePath).fileName();
  QString mimeType = QMimeDatabase().mimeTypeForFile(job.filePath).name();

  imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mimeType));
  imagePart.setHeader(
      QNetworkRequest::ContentDispositionHeader,
      QVariant("form-data; name=\"photo\"; filename=\"" + fileName + "\""));
  imagePart.setBodyDevice(file);
  file->setParent(multiPart);
  multiPart->append(imagePart);

  // 2. NEW: Server Path Part (if present)
  if (!m_targetServerPath.isEmpty()) {
    QHttpPart pathPart;
    pathPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"path\""));
    pathPart.setBody(m_targetServerPath.toUtf8());
    multiPart->append(pathPart);
  }

  // 3. Metadata Part
  Photo p(job.filePath);
  QJsonObject metaJson;
  metaJson["copyright"] = p.getCopyright();
  metaJson["gps"] = p.getGpsDecimalString();

  QHttpPart metaPart;
  metaPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     QVariant("form-data; name=\"metadata\""));
  metaPart.setBody(QJsonDocument(metaJson).toJson(QJsonDocument::Compact));
  multiPart->append(metaPart);

  QNetworkReply *reply = m_netManager->post(request, multiPart);
  multiPart->setParent(reply);

  connect(reply, &QNetworkReply::uploadProgress, this,
          &UploadManager::onUploadProgress);

  emit uploadProgress(m_processedFiles + 1, m_totalFiles, 0);
}

void UploadManager::onUploadProgress(qint64 bytesSent, qint64 bytesTotal) {
  if (bytesTotal > 0 && m_state == UPLOADING) {
    int percent = static_cast<int>((bytesSent * 100) / bytesTotal);
    emit uploadProgress(m_processedFiles + 1, m_totalFiles, percent);
  }
}

void UploadManager::onNetworkFinished(QNetworkReply *reply) {
  reply->deleteLater();

  int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  QString path = reply->request().url().path();

  // Auth Refresh Logic
  if (statusCode == 401 && !path.endsWith("/login") &&
      !path.endsWith("/refresh")) {
    if (!m_refreshToken.isEmpty() && !m_isRefreshing) {
      qDebug() << "Token expired (401). Refreshing...";
      performTokenRefresh();
    } else {
      m_state = IDLE;
      emit loginFailed("Session expired. Please re-login.");
    }
    return;
  }

  QByteArray data = reply->readAll();

  // Refresh
  if (path.endsWith("/refresh")) {
    m_isRefreshing = false;
    if (statusCode == 200) {
      QJsonDocument doc = QJsonDocument::fromJson(data);
      m_jwtToken = doc.object()["token"].toString();
      qDebug() << "Token refreshed. Retrying...";
      retryLastAction();
    } else {
      m_state = IDLE;
      emit loginFailed("Session refresh failed.");
    }
    return;
  }

  // Login
  if (m_state == LOGGING_IN) {
    if (statusCode == 200) {
      QJsonDocument doc = QJsonDocument::fromJson(data);
      QJsonObject obj = doc.object();
      if (obj.contains("token")) {
        m_jwtToken = obj["token"].toString();
        m_refreshToken = obj["refreshToken"].toString();
        m_state = IDLE;
        emit loginSuccessful();
      } else {
        emit loginFailed("Invalid server response");
      }
    } else {
      emit loginFailed("Login failed: " + QString::number(statusCode));
    }
    return;
  }

  // Upload
  if (m_state == UPLOADING) {
    UploadJob job = m_queue.dequeue();
    m_processedFiles++;

    if (reply->error() == QNetworkReply::NoError) {
      emit fileUploaded(QFileInfo(job.filePath).fileName(), true);
    } else {
      qWarning() << "Upload Error:" << reply->errorString();
      emit fileUploaded(QFileInfo(job.filePath).fileName(), false);
    }

    startNextUpload();
  }
}

void UploadManager::performTokenRefresh() {
  m_isRefreshing = true;
  QUrl url(m_serverUrl + "/refresh");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["refreshToken"] = m_refreshToken;
  m_netManager->post(request, QJsonDocument(json).toJson());
}

void UploadManager::retryLastAction() {
  if (m_state == UPLOADING) {
    startNextUpload();
  }
}