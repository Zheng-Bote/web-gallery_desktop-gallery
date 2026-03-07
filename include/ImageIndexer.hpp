/**
 * SPDX-FileComment: Background task for scanning folders and indexing images
 * SPDX-FileType: HEADER
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file ImageIndexer.hpp
 * @brief Background task for scanning folders and indexing images.
 * @version 1.0.0
 * @date 2026-03-07
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <QObject>
#include <QRunnable>
#include <QString>

/**
 * @class ImageIndexer
 * @brief Background task for scanning folders and indexing images.
 *
 * This class runs in a separate thread (via QThreadPool) to avoid blocking
 * the UI during large folder scans. It populates the database with image
 * metadata.
 */
class ImageIndexer : public QObject, public QRunnable {
  Q_OBJECT
public:
  /**
   * @brief Constructor.
   * @param folderPath The path to the folder to scan.
   */
  explicit ImageIndexer(const QString &folderPath);

  /**
   * @brief The main execution loop of the runnable.
   */
  void run() override;

signals:
  /**
   * @brief Signal emitted to report progress.
   * @param count The number of images processed so far.
   */
  void progress(int count);

  /**
   * @brief Signal emitted when the indexing process is finished.
   */
  void finished();

private:
  QString m_folderPath;
};