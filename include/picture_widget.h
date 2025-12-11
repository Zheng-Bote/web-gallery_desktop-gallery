#pragma once

#include <QCloseEvent>
#include <QMenu>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QWidget>

#include "rz_photo.hpp"

namespace Ui {
class PictureWidget;
}

class PictureWidget : public QWidget {
  Q_OBJECT

public:
  explicit PictureWidget(QWidget *parent = nullptr);
  ~PictureWidget();

  void setImage(QString pathToFile);

  // NEU: Signal, wenn sich das Bild geändert hat
signals:
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

  void resizeEvent(QResizeEvent *event);
  void closeEvent(QCloseEvent *event);

  void resizePicture();

  bool dataModified_bool{false};
  QString pathToImage;
  QPixmap *picture;

  void updateSyncTarget(QTableWidget *targetTable, const QString &targetKey,
                        const QString &newValue);

  const QList<QString> validMetaImageTypes = {"jpg", "jpeg", "png", "webp",
                                              "tiff"};
  bool checkValidMetaImg();
  void disableMetaTabs(int tab);
  const void readSrcExif();
  void markIptc(QString searchFor);
  void markExif(QString searchFor);
  const void readSrcIptc();

  const void readSrcXmp();
  void markXmp(QString searchFor);

  void mapMetaData(const QString &pathToFile, const QString &key,
                   const QString &value, const QString &source);
};
