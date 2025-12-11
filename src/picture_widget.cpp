/**
 * @file picture_widget.cpp
 * @author ZHENG Robert
 * @brief Main class for the picture detail widget
 */
#include "picture_widget.h"
#include "rz_metadata.hpp"
#include "ui_picture_widget.h"

#include <QCloseEvent>
#include <QDebug>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>

// Exiv2 und QtConcurrent Includes
#include <QFuture>
#include <QFutureWatcher>
#include <QThread>
#include <QtConcurrent>
#include <exiv2/exiv2.hpp>

PictureWidget::PictureWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::PictureWidget) {
  ui->setupUi(this);

  // Initialisierung von Exiv2
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  ui->closeBtn->setIcon(
      QIcon(":/resources/img/icons8-arrow-pointing-left-50.png"));
  ui->exportSrcToWebp_Btn->setIcon(
      QIcon(":/resources/img/icons8-send-file-50.png"));

  picture = nullptr;

  createRotateMenu();
  createExportMenu();
}

PictureWidget::~PictureWidget() {
  delete ui;

  if (picture) {
    delete picture;
    picture = nullptr;
  }
}

void PictureWidget::setImage(QString pathToFile) {
  pathToImage = pathToFile;
  QFile srcFile(pathToFile);
  QFileInfo fileInfo(srcFile.fileName());

  if (picture) {
    delete picture;
  }
  picture = new QPixmap(pathToImage);

  QString pictureData = fileInfo.fileName() + " \n(" +
                        QString::number(picture->width()) + "x" +
                        QString::number(picture->height()) + " " +
                        QString::number(fileInfo.size() / 1024) + " KiB)";
  ui->filename_label->setText(pictureData);
  ui->filename_label->setToolTip(pathToFile);

  ui->picture_label->setPixmap(picture->scaled(ui->picture_label->size(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));

  QFuture<void> futureExif =
      QtConcurrent::run(&PictureWidget::readSrcExif, this);
  QFuture<void> futureIptc =
      QtConcurrent::run(&PictureWidget::readSrcIptc, this);
  QFuture<void> futureXmp = QtConcurrent::run(&PictureWidget::readSrcXmp, this);

  ui->tabWidget->adjustSize();
  ui->tabWidget->setCurrentWidget(0);
}

void PictureWidget::on_closeBtn_clicked() { this->close(); }

void PictureWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  resizePicture();
}

void PictureWidget::closeEvent(QCloseEvent *event) {
  if (dataModified_bool) {
    QMessageBox::StandardButton response;

    response = QMessageBox::question(
        this, tr("Confirmation"),
        tr("Are you sure you want to exit without saving data?"),
        QMessageBox::Yes | QMessageBox::No);

    if (response == QMessageBox::Yes) {
      event->accept();
    } else {
      event->ignore();
    }
  } else {
    event->accept();
  }
}

void PictureWidget::resizePicture() {
  if (picture && !picture->isNull()) {
    ui->picture_label->setPixmap(picture->scaled(ui->picture_label->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
  }
}

bool PictureWidget::checkValidMetaImg() {
  bool ret{false};
  Photo *photo = new Photo(pathToImage);
  QString photoExtension = photo->getSuffix().toLower();

  // Hinweis: Wir nutzen hier die statische Liste aus Photo
  if (Photo::metaSupportedTypes.contains(photoExtension, Qt::CaseInsensitive)) {
    ret = true;
  }

  delete photo;
  return ret;
}

void PictureWidget::disableMetaTabs(int tab) {
  ui->tabWidget->setTabVisible(tab, false);
}

const void PictureWidget::readSrcExif() {
  if (!checkValidMetaImg()) {
    disableMetaTabs(1);
    return;
  }

  QFile file(pathToImage);
  auto exif_image =
      Exiv2::ImageFactory::open(file.fileName().toUtf8().toStdString());

  QFont font_11_bold("Times New Roman", 11);
  font_11_bold.setBold(true);

  ui->exifTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->exifTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  ui->exifTableWidget->setRowCount(0);
  ui->exifTableWidget->setColumnCount(3);

  QTableWidgetItem *tlbCol1 = new QTableWidgetItem();
  tlbCol1->setText("EXIF key");
  tlbCol1->setBackground(Qt::lightGray);
  tlbCol1->setFont(font_11_bold);
  tlbCol1->setTextAlignment(Qt::AlignCenter);
  ui->exifTableWidget->setHorizontalHeaderItem(0, tlbCol1);

  QTableWidgetItem *tlbCol2 = new QTableWidgetItem();
  tlbCol2->setText("EXIF value");
  tlbCol2->setBackground(Qt::lightGray);
  tlbCol2->setFont(font_11_bold);
  tlbCol2->setTextAlignment(Qt::AlignCenter);
  ui->exifTableWidget->setHorizontalHeaderItem(1, tlbCol2);

  QTableWidgetItem *tlbCol3 = new QTableWidgetItem();
  tlbCol3->setText("EXIF description");
  tlbCol3->setBackground(Qt::lightGray);
  tlbCol3->setFont(font_11_bold);
  tlbCol3->setTextAlignment(Qt::AlignCenter);
  ui->exifTableWidget->setHorizontalHeaderItem(2, tlbCol3);

  exif_image->readMetadata();

  Exiv2::ExifData &exifData = exif_image->exifData();

  if (!exifData.empty()) {
    auto end = exifData.end();
    for (auto md = exifData.begin(); md != end; ++md) {
      ui->exifTableWidget->insertRow(ui->exifTableWidget->rowCount());
      ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1, 0,
                                   new QTableWidgetItem(md->key().c_str()));

      QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
      tlbCol2val->setText(md->value().toString().c_str());
      ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1, 1,
                                   tlbCol2val);

      if (RzMetadata::exifTagDescriptions.contains(md->key().c_str())) {
        QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
        tlbCol3val->setText(
            RzMetadata::exifTagDescriptions.value(md->key().c_str(), ""));
        ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1, 2,
                                     tlbCol3val);
      }
    }
  }
  // Keine Default-Rows mehr anzeigen, nur echte Daten.
  file.close();
}

void PictureWidget::markExif(QString searchFor) {
  // Diese Funktion wird in der aktuellen Logik nicht mehr zwingend benötigt,
  // da wir keine Defaults mehr "markieren".
  // Der Vollständigkeit halber bleibt sie drin oder kann entfernt werden.
}

const void PictureWidget::readSrcIptc() {
  if (!checkValidMetaImg()) {
    disableMetaTabs(2);
    return;
  }
  QFile file(pathToImage);
  auto iptc_image =
      Exiv2::ImageFactory::open(file.fileName().toUtf8().toStdString());

  QFont font_11_bold("Times New Roman", 11);
  font_11_bold.setBold(true);

  ui->iptcTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->iptcTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  ui->iptcTableWidget->setRowCount(0);
  ui->iptcTableWidget->setColumnCount(3);

  QTableWidgetItem *tlbCol1 = new QTableWidgetItem();
  tlbCol1->setText("IPTC key");
  tlbCol1->setBackground(Qt::lightGray);
  tlbCol1->setFont(font_11_bold);
  tlbCol1->setTextAlignment(Qt::AlignCenter);
  ui->iptcTableWidget->setHorizontalHeaderItem(0, tlbCol1);

  QTableWidgetItem *tlbCol2 = new QTableWidgetItem();
  tlbCol2->setText("IPTC value");
  tlbCol2->setBackground(Qt::lightGray);
  tlbCol2->setFont(font_11_bold);
  tlbCol2->setTextAlignment(Qt::AlignCenter);
  ui->iptcTableWidget->setHorizontalHeaderItem(1, tlbCol2);

  QTableWidgetItem *tlbCol3 = new QTableWidgetItem();
  tlbCol3->setText("IPTC description");
  tlbCol3->setBackground(Qt::lightGray);
  tlbCol3->setFont(font_11_bold);
  tlbCol3->setTextAlignment(Qt::AlignCenter);
  ui->iptcTableWidget->setHorizontalHeaderItem(2, tlbCol3);

  iptc_image->readMetadata();

  Exiv2::IptcData &iptcData = iptc_image->iptcData();

  if (!iptcData.empty()) {
    auto end = iptcData.end();
    for (auto md = iptcData.begin(); md != end; ++md) {
      ui->iptcTableWidget->insertRow(ui->iptcTableWidget->rowCount());
      ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1, 0,
                                   new QTableWidgetItem(md->key().c_str()));

      QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
      tlbCol2val->setText(md->value().toString().c_str());
      ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1, 1,
                                   tlbCol2val);

      if (RzMetadata::iptcTagDescriptions.contains(md->key().c_str())) {
        QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
        tlbCol3val->setText(
            RzMetadata::iptcTagDescriptions.value(md->key().c_str(), ""));
        ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1, 2,
                                     tlbCol3val);
      }
    }
  }
  file.close();
}

const void PictureWidget::readSrcXmp() {
  if (!checkValidMetaImg()) {
    disableMetaTabs(3);
    return;
  }
  QFile file(pathToImage);
  auto xmp_image =
      Exiv2::ImageFactory::open(file.fileName().toUtf8().toStdString());

  QFont font_11_bold("Times New Roman", 11);
  font_11_bold.setBold(true);

  ui->xmpTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->xmpTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  ui->xmpTableWidget->setRowCount(0);
  ui->xmpTableWidget->setColumnCount(3);

  QTableWidgetItem *tlbCol1 = new QTableWidgetItem();
  tlbCol1->setText("XMP key");
  tlbCol1->setBackground(Qt::lightGray);
  tlbCol1->setFont(font_11_bold);
  tlbCol1->setTextAlignment(Qt::AlignCenter);
  ui->xmpTableWidget->setHorizontalHeaderItem(0, tlbCol1);

  QTableWidgetItem *tlbCol2 = new QTableWidgetItem();
  tlbCol2->setText("XMP value");
  tlbCol2->setBackground(Qt::lightGray);
  tlbCol2->setFont(font_11_bold);
  tlbCol2->setTextAlignment(Qt::AlignCenter);
  ui->xmpTableWidget->setHorizontalHeaderItem(1, tlbCol2);

  QTableWidgetItem *tlbCol3 = new QTableWidgetItem();
  tlbCol3->setText("XMP description");
  tlbCol3->setBackground(Qt::lightGray);
  tlbCol3->setFont(font_11_bold);
  tlbCol3->setTextAlignment(Qt::AlignCenter);
  ui->xmpTableWidget->setHorizontalHeaderItem(2, tlbCol3);

  xmp_image->readMetadata();

  Exiv2::XmpData &xmpData = xmp_image->xmpData();

  if (!xmpData.empty()) {
    auto end = xmpData.end();
    for (auto md = xmpData.begin(); md != end; ++md) {
      ui->xmpTableWidget->insertRow(ui->xmpTableWidget->rowCount());
      ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1, 0,
                                  new QTableWidgetItem(md->key().c_str()));

      QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
      tlbCol2val->setText(md->value().toString().c_str());
      ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1, 1,
                                  tlbCol2val);

      if (RzMetadata::xmpTagDescriptions.contains(md->key().c_str())) {
        QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
        tlbCol3val->setText(
            RzMetadata::xmpTagDescriptions.value(md->key().c_str(), ""));
        ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1, 2,
                                    tlbCol3val);
      }
    }
  }
  file.close();

  ui->xmpTableWidget->resizeColumnsToContents();
  ui->xmpTableWidget->resizeRowsToContents();
}

void PictureWidget::markXmp(QString searchFor) {
  // Nicht mehr benötigt
}

void PictureWidget::mapMetaData(const QString &pathToFile, const QString &key,
                                const QString &_value, const QString &source) {
  // Veraltet, wird durch updateSyncTarget und on_saveBtn_clicked ersetzt.
}

void PictureWidget::markIptc(QString searchFor) {
  // Nicht mehr benötigt
}

void PictureWidget::createRotateMenu() {
  ui->rotateSrcImg_Btn->setIcon(
      QIcon(":/resources/img/icons8-available-updates-50.png"));

  rotateMnu = new QMenu();

  rotate_90 = new QAction(QIcon(":/resources/img/icons8-rotate-right-50.png"),
                          tr("rotate") + " 90°", this);
  connect(rotate_90, &QAction::triggered, this,
          [this] { PictureWidget::rotateSrcImg(90); });
  rotateMnu->addAction(rotate_90);

  rotate_120 = new QAction(QIcon(":/resources/img/icons8-rotate-right-50.png"),
                           tr("rotate") + " 120°", this);
  connect(rotate_120, &QAction::triggered, this,
          [this] { PictureWidget::rotateSrcImg(120); });
  rotateMnu->addAction(rotate_120);

  rotateMnu->addSeparator();

  rotate_m90 = new QAction(QIcon(":/resources/img/icons8-rotate-left-50.png"),
                           tr("rotate") + " -90°", this);
  connect(rotate_m90, &QAction::triggered, this,
          [this] { PictureWidget::rotateSrcImg(-90); });
  rotateMnu->addAction(rotate_m90);

  rotate_m120 = new QAction(QIcon(":/resources/img/icons8-rotate-left-50.png"),
                            tr("rotate") + " -120°", this);
  connect(rotate_m120, &QAction::triggered, this,
          [this] { PictureWidget::rotateSrcImg(-120); });
  rotateMnu->addAction(rotate_m120);

  ui->rotateSrcImg_Btn->setMenu(rotateMnu);
}

void PictureWidget::createExportMenu() {
  exportMnu = new QMenu();
  exportMnu->setIcon(QIcon(":/resources/img/icons8-send-file-50.png"));

  auto addExportAction = [this](int size) {
    QAction *act =
        new QAction(QIcon(":/resources/img/icons8-ausgang-48.png"),
                    tr("export to size") + " " + QString::number(size), this);
    connect(act, &QAction::triggered, this,
            [this, size] { PictureWidget::exportSrcImgToWebP(size); });
    exportMnu->addAction(act);
  };

  addExportAction(480);
  addExportAction(640);
  addExportAction(800);
  addExportAction(1024);
  addExportAction(1280);

  exportMnu->addSeparator();

  webp_size_all = new QAction(QIcon(":/resources/img/icons8-send-file-50.png"),
                              tr("export to all sizes"), this);
  connect(webp_size_all, &QAction::triggered, this,
          [this] { PictureWidget::exportSrcImgToWebpThread(); });
  exportMnu->addAction(webp_size_all);

  exportMnu->addSeparator();

  webp_oversizeAct = new QAction(tr("increase too small picture"), this);
  webp_oversizeAct->setCheckable(true);
  webp_oversizeAct->setChecked(true);
  exportMnu->addAction(webp_oversizeAct);

  webp_overwriteWebpAct = new QAction(tr("overwrite existing WebP"), this);
  webp_overwriteWebpAct->setCheckable(true);
  webp_overwriteWebpAct->setChecked(true);
  exportMnu->addAction(webp_overwriteWebpAct);

  webp_watermarkWebpAct = new QAction(tr("watermark WebP"), this);
  webp_watermarkWebpAct->setCheckable(true);
  webp_watermarkWebpAct->setChecked(true);
  exportMnu->addAction(webp_watermarkWebpAct);

  ui->exportSrcToWebp_Btn->setMenu(exportMnu);
}

void PictureWidget::_on_resizeSmallerImage_Btn_clicked() {
  resize(width() - 100, height() - 100);
}

void PictureWidget::_on_resizeBiggerImage_Btn_clicked() {
  resize(width() + 100, height() + 100);
}

// --- SYNC IMPLEMENTATION ---

void PictureWidget::updateSyncTarget(QTableWidget *targetTable,
                                     const QString &targetKey,
                                     const QString &newValue) {
  if (!targetTable)
    return;

  auto items = targetTable->findItems(targetKey, Qt::MatchExactly);

  if (items.isEmpty()) {
    // Key existiert noch nicht -> Neue Zeile anlegen
    int newRow = targetTable->rowCount();
    targetTable->insertRow(newRow);

    // Spalte 0: Key (Schreibgeschützt)
    QTableWidgetItem *keyItem = new QTableWidgetItem(targetKey);
    keyItem->setFlags(keyItem->flags() ^ Qt::ItemIsEditable);
    targetTable->setItem(newRow, 0, keyItem);

    // Spalte 1: Value
    targetTable->setItem(newRow, 1, new QTableWidgetItem(newValue));

    // Spalte 2: Description (Automatisch ermitteln)
    QString desc = "";
    if (targetKey.startsWith("Exif.")) {
      desc = RzMetadata::exifTagDescriptions.value(targetKey, "");
    } else if (targetKey.startsWith("Iptc.")) {
      desc = RzMetadata::iptcTagDescriptions.value(targetKey, "");
    } else if (targetKey.startsWith("Xmp.")) {
      desc = RzMetadata::xmpTagDescriptions.value(targetKey, "");
    }

    QTableWidgetItem *descItem = new QTableWidgetItem(desc);
    descItem->setFlags(descItem->flags() ^ Qt::ItemIsEditable);
    targetTable->setItem(newRow, 2, descItem);

    targetTable->resizeColumnsToContents();
  } else {
    for (auto item : items) {
      if (item->column() == 0) { // Key Spalte
        int row = item->row();
        if (!targetTable->item(row, 1)) {
          targetTable->setItem(row, 1, new QTableWidgetItem(newValue));
        } else {
          targetTable->item(row, 1)->setText(newValue);
        }
      }
    }
    targetTable->resizeColumnsToContents();
  }
}

void PictureWidget::on_exifTableWidget_itemDoubleClicked(
    QTableWidgetItem *item) {
  bool ok;
  int row = item->row();
  QString key = ui->exifTableWidget->item(row, 0)->text();
  QString currentVal = ui->exifTableWidget->item(row, 1)
                           ? ui->exifTableWidget->item(row, 1)->text()
                           : "";

  QString text = QInputDialog::getText(this, tr("Edit Metadata"),
                                       tr("Please enter the new value for") +
                                           " " + key + ":",
                                       QLineEdit::Normal, currentVal, &ok);
  if (ok) {
    // UI Update (kein File-Write)
    if (!ui->exifTableWidget->item(row, 1))
      ui->exifTableWidget->setItem(row, 1, new QTableWidgetItem(text));
    else
      ui->exifTableWidget->item(row, 1)->setText(text);

    dataModified_bool = true;

    // Live Sync
    if (RzMetadata::exif_to_xmp.contains(key))
      updateSyncTarget(ui->xmpTableWidget, RzMetadata::exif_to_xmp[key], text);

    if (RzMetadata::exif_to_iptc.contains(key))
      updateSyncTarget(ui->iptcTableWidget, RzMetadata::exif_to_iptc[key],
                       text);
  }
}

void PictureWidget::on_iptcTableWidget_itemDoubleClicked(
    QTableWidgetItem *item) {
  bool ok;
  int row = item->row();
  QString key = ui->iptcTableWidget->item(row, 0)->text();
  QString currentVal = ui->iptcTableWidget->item(row, 1)
                           ? ui->iptcTableWidget->item(row, 1)->text()
                           : "";

  QString text = QInputDialog::getText(this, tr("Edit Metadata"),
                                       tr("Please enter the new value for") +
                                           " " + key + ":",
                                       QLineEdit::Normal, currentVal, &ok);
  if (ok) {
    if (!ui->iptcTableWidget->item(row, 1))
      ui->iptcTableWidget->setItem(row, 1, new QTableWidgetItem(text));
    else
      ui->iptcTableWidget->item(row, 1)->setText(text);

    dataModified_bool = true;

    // Live Sync
    if (RzMetadata::iptc_to_exif.contains(key))
      updateSyncTarget(ui->exifTableWidget, RzMetadata::iptc_to_exif[key],
                       text);

    if (RzMetadata::iptc_to_xmp.contains(key))
      updateSyncTarget(ui->xmpTableWidget, RzMetadata::iptc_to_xmp[key], text);
  }
}

void PictureWidget::on_xmpTableWidget_itemDoubleClicked(
    QTableWidgetItem *item) {
  bool ok;
  int row = item->row();
  QString key = ui->xmpTableWidget->item(row, 0)->text();
  QString currentVal = ui->xmpTableWidget->item(row, 1)
                           ? ui->xmpTableWidget->item(row, 1)->text()
                           : "";

  QString text = QInputDialog::getText(this, tr("Edit Metadata"),
                                       tr("Please enter the new value for") +
                                           " " + key + ":",
                                       QLineEdit::Normal, currentVal, &ok);
  if (ok) {
    if (!ui->xmpTableWidget->item(row, 1))
      ui->xmpTableWidget->setItem(row, 1, new QTableWidgetItem(text));
    else
      ui->xmpTableWidget->item(row, 1)->setText(text);

    dataModified_bool = true;

    // Live Sync
    if (RzMetadata::xmp_to_exif.contains(key))
      updateSyncTarget(ui->exifTableWidget, RzMetadata::xmp_to_exif[key], text);

    if (RzMetadata::xmp_to_iptc.contains(key))
      updateSyncTarget(ui->iptcTableWidget, RzMetadata::xmp_to_iptc[key], text);
  }
}

void PictureWidget::rotateSrcImg(int val) {
  Photo *photo = new Photo(pathToImage);
  photo->rotateImage(val);
  delete photo;

  if (picture)
    delete picture;
  picture = new QPixmap(pathToImage);
  resizePicture();

  // Signal emitten für Thumbnail Refresh
  emit imageChanged(pathToImage);
}

void PictureWidget::exportSrcImgToWebpThread() {
  bool oknok{false};
  Photo phot(pathToImage);

  if (webp_oversizeAct->isChecked())
    phot.setOversizeSmallerPicture(true);
  if (webp_overwriteWebpAct->isChecked())
    phot.setOverwriteExistingWebp(true);
  if (webp_watermarkWebpAct->isChecked())
    phot.setWatermarkWebp(true);

  QFuture<bool> future = QtConcurrent::run(&Photo::convertImages, phot, 75);
  oknok = future.result();

  QMessageBox msgBox(this);
  msgBox.setWindowTitle(oknok ? tr("Success") : tr("Error"));
  msgBox.setIcon(oknok ? QMessageBox::Information : QMessageBox::Warning);
  msgBox.setText(tr("WebP export to subfolder WebP") +
                 (oknok ? " " + tr("successful") : " " + tr("failed")));
  msgBox.setInformativeText("<i>" + pathToImage + "</i>");
  msgBox.setFixedWidth(900);
  msgBox.exec();
}

void PictureWidget::exportSrcImgToWebP(int size) {
  Photo photo(pathToImage);

  if (webp_oversizeAct->isChecked())
    photo.setOversizeSmallerPicture(true);
  if (webp_overwriteWebpAct->isChecked())
    photo.setOverwriteExistingWebp(true);
  if (webp_watermarkWebpAct->isChecked())
    photo.setWatermarkWebp(true);

  bool result = photo.convertImage(size);

  QMessageBox msgBox(this);
  msgBox.setWindowTitle(result ? tr("Success") : tr("Error"));
  msgBox.setIcon(result ? QMessageBox::Information : QMessageBox::Warning);
  msgBox.setText(tr("WebP export") +
                 (result ? " " + tr("successful") : " " + tr("failed")));
  msgBox.setInformativeText("<i>" + pathToImage + "</i>");
  msgBox.setFixedWidth(900);
  msgBox.exec();
}

void PictureWidget::on_tabWidget_tabBarClicked(int index) {
  ui->exifTableWidget->resizeColumnsToContents();
  ui->iptcTableWidget->resizeColumnsToContents();
  ui->xmpTableWidget->resizeColumnsToContents();
}

void PictureWidget::on_saveBtn_clicked() {
  // 1. Check
  Photo photoCheck(pathToImage);
  if (!photoCheck.hasMetadataSupport()) {
    QMessageBox::warning(this, tr("Save Error"),
                         tr("This file format (%1) does not support metadata.")
                             .arg(photoCheck.getSuffix()));
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  try {
    // 2. Open
    auto image = Exiv2::ImageFactory::open(pathToImage.toStdString());
    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    Exiv2::IptcData &iptcData = image->iptcData();
    Exiv2::XmpData &xmpData = image->xmpData();

    // Helper
    auto saveTableToData = [](QTableWidget *table, auto &metaContainer) {
      for (int i = 0; i < table->rowCount(); ++i) {
        QString key = table->item(i, 0)->text();
        QString val = "";
        if (table->item(i, 1)) {
          val = table->item(i, 1)->text();
        }

        if (!key.isEmpty()) {
          try {
            if (val.isEmpty()) {
              metaContainer[key.toStdString()] = ""; // Leerstring schreiben
            } else {
              metaContainer[key.toStdString()] = val.toStdString();
            }
          } catch (Exiv2::Error &e) {
            qWarning() << "Error writing key" << key << ":" << e.what();
          }
        }
      }
    };

    // 3. Save Tables
    saveTableToData(ui->exifTableWidget, exifData);
    saveTableToData(ui->iptcTableWidget, iptcData);
    saveTableToData(ui->xmpTableWidget, xmpData);

    // 4. Write
    image->writeMetadata();

    dataModified_bool = false;
    QApplication::restoreOverrideCursor();

    // 5. Signal
    emit imageChanged(pathToImage);

  } catch (Exiv2::Error &e) {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, tr("Save Error"),
                          tr("Failed to save metadata:\n%1").arg(e.what()));
  } catch (...) {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, tr("Save Error"),
                          tr("Unknown error occurred while saving."));
  }
}