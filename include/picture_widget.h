/**
 * SPDX-FileComment: Window for viewing and editing details of a single image
 * SPDX-FileType: HEADER
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file picture_widget.h
 * @brief Window for viewing and editing details of a single image.
 * @version 1.1.0
 * @date 2026-10-31
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once

#include <QCloseEvent>
#include <QHash>
#include <QMenu>
#include <QPair>
#include <QPixmap>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "rz_photo.hpp"

class QWebEngineView;

namespace Ui {
class PictureWidget;
}

/**
 * @class PictureWidget
 * @brief Window for viewing and editing details of a single image.
 *
 * Provides functionalities to view image content, edit detailed metadata
 * (EXIF/IPTC/XMP), view location on a map, and perform single-image operations.
 */
class PictureWidget : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent The parent widget.
   */
  explicit PictureWidget(QWidget *parent = nullptr);
  ~PictureWidget();

  /**
   * @brief Set the image to display.
   * @param pathToFile Absolute path to the image file.
   */
  void setImage(QString pathToFile);

  // NEW: Signal when the image has changed
signals:
  /** @brief Signal emitted when the image data is modified. */
  void imageChanged(const QString &path);

private slots:
  void on_closeBtn_clicked();
  void on_saveBtn_clicked();

  void _on_resizeSmallerImage_Btn_clicked();
  void _on_resizeBiggerImage_Btn_clicked();

  void on_exifTableWidget_itemDoubleClicked(QTableWidgetItem *item);
  void on_iptcTableWidget_itemDoubleClicked(QTableWidgetItem *item);
  void on_xmpTableWidget_itemDoubleClicked(QTableWidgetItem *item);

  void rotateSrcImg(int val);
  void exportSrcImgToWebpThread();
  void exportSrcImgToWebP(int size);
  void exportSrcImgToAvifThread();
  void exportSrcImgToAvif(int size);

  void on_tabWidget_tabBarClicked(int index);

private:
  Ui::PictureWidget *ui;

  void createRotateMenu();
  QMenu *rotateMnu;
  QAction *rotate_90;
  QAction *rotate_m90;
  QAction *rotate_120;
  QAction *rotate_m120;

  void createExportMenu();
  QMenu *exportMnu;
  QAction *webp_oversizeAct;
  QAction *webp_overwriteWebpAct;
  QAction *webp_watermarkWebpAct;
  QAction *webp_size_all;
  QAction *webp_size_480;
  QAction *webp_size_640;
  QAction *webp_size_800;
  QAction *webp_size_1024;
  QAction *webp_size_1280;

  void createAvifExportMenu();
  QMenu *exportAvifMnu;
  QAction *avif_oversizeAct;
  QAction *avif_overwriteAvifAct;
  QAction *avif_watermarkAvifAct;
  QAction *avif_size_all;

  void resizeEvent(QResizeEvent *event);
  void closeEvent(QCloseEvent *event);

  void resizePicture();

  bool dataModified_bool{false};
  QString pathToImage;
  QPixmap picture;
  QTimer *m_resizeTimer{nullptr};

  QWebEngineView *m_mapView;
  void loadMap();

  void updateSyncTarget(QTableWidget *targetTable, const QString &targetKey,
                        const QString &newValue);

  const QList<QString> validMetaImageTypes = {"jpg", "jpeg", "png", "webp",
                                              "tiff"};
  bool checkValidMetaImg();
  void disableMetaTabs(int tab);
  void loadMetadataAsync();
  void populateMetaTable(QTableWidget *table, const QString &headerPrefix,
                         const QVector<QPair<QString, QString>> &entries,
                         const QHash<QString, QString> &descriptions,
                         bool resizeRowsAndCols = false);
  const void readSrcExif();
  void markIptc(QString searchFor);
  void markExif(QString searchFor);
  const void readSrcIptc();

  const void readSrcXmp();
  void markXmp(QString searchFor);

  void mapMetaData(const QString &pathToFile, const QString &key,
                   const QString &value, const QString &source);
};
