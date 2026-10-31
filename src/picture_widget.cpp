/**
 * SPDX-FileComment: Main class for the picture detail widget
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file picture_widget.cpp
 * @brief Main class for the picture detail widget.
 * @version 1.0.0
 * @date 2026-03-07
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#include "picture_widget.h"
#include "rz_metadata.hpp"
#include "ui_picture_widget.h"

#include <QCloseEvent>
#include <QDebug>
#include <QFileInfo>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include <QPointF>
#include <QWebEngineView>

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
  ui->exportSrcToAvif_Btn->setIcon(
      QIcon(":/resources/img/icons8-send-file-50.png"));

  m_resizeTimer = new QTimer(this);
  m_resizeTimer->setSingleShot(true);
  m_resizeTimer->setInterval(100);
  connect(m_resizeTimer, &QTimer::timeout, this, &PictureWidget::resizePicture);

  // --- MAP VIEW INITIALISIEREN ---
  m_mapView = new QWebEngineView(this);
  // Wir fügen den Tab programmatisch hinzu (Index 3 oder 4, je nachdem was im
  // UI File ist)
  ui->tabWidget->addTab(m_mapView, tr("GPS Map"));

  createRotateMenu();
  createExportMenu();
  createAvifExportMenu();
}

PictureWidget::~PictureWidget() { delete ui; }

void PictureWidget::setImage(QString pathToFile) {
  pathToImage = pathToFile;
  QFile srcFile(pathToFile);
  QFileInfo fileInfo(srcFile.fileName());

  picture = QPixmap(pathToImage);

  QString pictureData = fileInfo.fileName() + " \n(" +
                        QString::number(picture.width()) + "x" +
                        QString::number(picture.height()) + " " +
                        QString::number(fileInfo.size() / 1024) + " KiB)";
  ui->filename_label->setText(pictureData);
  ui->filename_label->setToolTip(pathToFile);

  ui->picture_label->setPixmap(picture.scaled(ui->picture_label->size(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));

  loadMetadataAsync();

  // Karte laden (Das machen wir im GUI Thread, da WebEngine das mag)
  loadMap();

  ui->tabWidget->adjustSize();
  ui->tabWidget->setCurrentWidget(0);
}

void PictureWidget::loadMap() {
  Photo p(pathToImage);
  QPointF gps = p.getGpsLatLon();

  // Index des Map-Tabs finden
  int mapIndex = ui->tabWidget->indexOf(m_mapView);

  // Check: Haben wir gültige Koordinaten?
  // (0.0/0.0 ist "Null Island" vor Afrika, meistens ungültig)
  if (qAbs(gps.x()) < 0.0001 && qAbs(gps.y()) < 0.0001) {
    // Tab deaktivieren oder Platzhalter anzeigen
    ui->tabWidget->setTabVisible(mapIndex, false); // Oder setEnabled(false)
    return;
  }

  // Tab sichtbar machen
  ui->tabWidget->setTabVisible(mapIndex, true);

  // HTML für EINEN Marker generieren
  QJsonObject markerObj;
  markerObj["lat"] = gps.x();
  markerObj["lng"] = gps.y();
  markerObj["title"] = QFileInfo(pathToImage).fileName().toHtmlEscaped();
  QString markerJson =
      QString::fromUtf8(QJsonDocument(markerObj).toJson(QJsonDocument::Compact));

  QString html = R"(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8">
        <style> 
            body, html, #map { height: 100%; margin: 0; padding: 0; } 
            /* Optional: Etwas CSS damit es hübsch aussieht */
        </style>
        <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" integrity="sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=" crossorigin="" />
        <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js" integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" crossorigin=""></script>
    </head>
    <body>
        <div id="map"></div>
        <script>
            var item = )" +
                 markerJson + R"(;
            // Karte auf die Koordinaten zentrieren, Zoom 15 (Detail)
            var map = L.map('map').setView([item.lat, item.lng], 15);

            L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                attribution: '&copy; OpenStreetMap contributors'
            }).addTo(map);

            // Marker setzen
            L.marker([item.lat, item.lng])
             .addTo(map)
             .bindPopup("<b>" + item.title + "</b>")
             .openPopup();
        </script>
    </body>
    </html>
    )";

  m_mapView->setHtml(html);
}

void PictureWidget::on_closeBtn_clicked() { this->close(); }

void PictureWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  if (!picture.isNull()) {
    ui->picture_label->setPixmap(picture.scaled(ui->picture_label->size(),
                                                Qt::KeepAspectRatio,
                                                Qt::FastTransformation));
    m_resizeTimer->start();
  }
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
  if (!picture.isNull()) {
    ui->picture_label->setPixmap(picture.scaled(ui->picture_label->size(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
  }
}

bool PictureWidget::checkValidMetaImg() {
  Photo photo(pathToImage);
  const QString photoExtension = photo.getSuffix().toLower();

  // Hinweis: Wir nutzen hier die statische Liste aus Photo
  return Photo::metaSupportedTypes.contains(photoExtension,
                                            Qt::CaseInsensitive);
}

void PictureWidget::disableMetaTabs(int tab) {
  ui->tabWidget->setTabVisible(tab, false);
}

void PictureWidget::populateMetaTable(
    QTableWidget *table, const QString &headerPrefix,
    const QVector<QPair<QString, QString>> &entries,
    const QHash<QString, QString> &descriptions, bool resizeRowsAndCols) {
  if (!table)
    return;

  QFont font_11_bold("Times New Roman", 11);
  font_11_bold.setBold(true);

  table->setUpdatesEnabled(false);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setRowCount(0);
  table->setColumnCount(3);

  auto createHeader = [&](const QString &title) {
    QTableWidgetItem *item = new QTableWidgetItem(title);
    item->setBackground(Qt::lightGray);
    item->setFont(font_11_bold);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
  };

  table->setHorizontalHeaderItem(0, createHeader(headerPrefix + " key"));
  table->setHorizontalHeaderItem(1, createHeader(headerPrefix + " value"));
  table->setHorizontalHeaderItem(2, createHeader(headerPrefix + " description"));

  table->setRowCount(entries.size());
  for (int row = 0; row < entries.size(); ++row) {
    const QString &key = entries[row].first;
    const QString &val = entries[row].second;

    table->setItem(row, 0, new QTableWidgetItem(key));
    table->setItem(row, 1, new QTableWidgetItem(val));

    if (descriptions.contains(key)) {
      table->setItem(row, 2,
                     new QTableWidgetItem(descriptions.value(key, "")));
    }
  }

  if (resizeRowsAndCols) {
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
  }
  table->setUpdatesEnabled(true);
}

void PictureWidget::loadMetadataAsync() {
  if (!checkValidMetaImg()) {
    disableMetaTabs(1);
    disableMetaTabs(2);
    disableMetaTabs(3);
    return;
  }

  ui->tabWidget->setTabVisible(1, true);
  ui->tabWidget->setTabVisible(2, true);
  ui->tabWidget->setTabVisible(3, true);

  const QString targetPath = pathToImage;

  (void)QtConcurrent::run([this, targetPath]() {
    QVector<QPair<QString, QString>> exifEntries;
    QVector<QPair<QString, QString>> iptcEntries;
    QVector<QPair<QString, QString>> xmpEntries;

    try {
      auto image =
          Exiv2::ImageFactory::open(targetPath.toUtf8().toStdString());
      if (image) {
        image->readMetadata();

        const Exiv2::ExifData &exifData = image->exifData();
        for (auto md = exifData.begin(); md != exifData.end(); ++md) {
          exifEntries.append({QString::fromStdString(md->key()),
                              QString::fromStdString(md->value().toString())});
        }

        const Exiv2::IptcData &iptcData = image->iptcData();
        for (auto md = iptcData.begin(); md != iptcData.end(); ++md) {
          iptcEntries.append({QString::fromStdString(md->key()),
                              QString::fromStdString(md->value().toString())});
        }

        const Exiv2::XmpData &xmpData = image->xmpData();
        for (auto md = xmpData.begin(); md != xmpData.end(); ++md) {
          xmpEntries.append({QString::fromStdString(md->key()),
                             QString::fromStdString(md->value().toString())});
        }
      }
    } catch (const Exiv2::Error &e) {
      qWarning() << "Exiv2 Error reading metadata for" << targetPath << ":"
                 << e.what();
    } catch (...) {
      qWarning() << "Unknown error reading metadata for" << targetPath;
    }

    QMetaObject::invokeMethod(
        this,
        [this, targetPath, exifEntries = std::move(exifEntries),
         iptcEntries = std::move(iptcEntries),
         xmpEntries = std::move(xmpEntries)]() {
          if (pathToImage != targetPath)
            return;
          populateMetaTable(ui->exifTableWidget, "EXIF", exifEntries,
                            RzMetadata::exifTagDescriptions, false);
          populateMetaTable(ui->iptcTableWidget, "IPTC", iptcEntries,
                            RzMetadata::iptcTagDescriptions, false);
          populateMetaTable(ui->xmpTableWidget, "XMP", xmpEntries,
                            RzMetadata::xmpTagDescriptions, true);
        },
        Qt::QueuedConnection);
  });
}

const void PictureWidget::readSrcExif() {
  if (!checkValidMetaImg()) {
    QMetaObject::invokeMethod(this, [this]() { disableMetaTabs(1); });
    return;
  }

  const QString targetPath = pathToImage;
  QVector<QPair<QString, QString>> exifEntries;
  try {
    auto exif_image =
        Exiv2::ImageFactory::open(targetPath.toUtf8().toStdString());
    exif_image->readMetadata();
    const Exiv2::ExifData &exifData = exif_image->exifData();
    for (auto md = exifData.begin(); md != exifData.end(); ++md) {
      exifEntries.append({QString::fromStdString(md->key()),
                          QString::fromStdString(md->value().toString())});
    }
  } catch (const Exiv2::Error &e) {
    qWarning() << "Exiv2 Error in readSrcExif:" << e.what();
  }

  QMetaObject::invokeMethod(
      this, [this, targetPath, exifEntries = std::move(exifEntries)]() {
        if (pathToImage != targetPath)
          return;
        populateMetaTable(ui->exifTableWidget, "EXIF", exifEntries,
                          RzMetadata::exifTagDescriptions, false);
      });
}

void PictureWidget::markExif(QString searchFor) {
  // Retained for interface compatibility.
}

const void PictureWidget::readSrcIptc() {
  if (!checkValidMetaImg()) {
    QMetaObject::invokeMethod(this, [this]() { disableMetaTabs(2); });
    return;
  }

  const QString targetPath = pathToImage;
  QVector<QPair<QString, QString>> iptcEntries;
  try {
    auto iptc_image =
        Exiv2::ImageFactory::open(targetPath.toUtf8().toStdString());
    iptc_image->readMetadata();
    const Exiv2::IptcData &iptcData = iptc_image->iptcData();
    for (auto md = iptcData.begin(); md != iptcData.end(); ++md) {
      iptcEntries.append({QString::fromStdString(md->key()),
                          QString::fromStdString(md->value().toString())});
    }
  } catch (const Exiv2::Error &e) {
    qWarning() << "Exiv2 Error in readSrcIptc:" << e.what();
  }

  QMetaObject::invokeMethod(
      this, [this, targetPath, iptcEntries = std::move(iptcEntries)]() {
        if (pathToImage != targetPath)
          return;
        populateMetaTable(ui->iptcTableWidget, "IPTC", iptcEntries,
                          RzMetadata::iptcTagDescriptions, false);
      });
}

const void PictureWidget::readSrcXmp() {
  if (!checkValidMetaImg()) {
    QMetaObject::invokeMethod(this, [this]() { disableMetaTabs(3); });
    return;
  }

  const QString targetPath = pathToImage;
  QVector<QPair<QString, QString>> xmpEntries;
  try {
    auto xmp_image =
        Exiv2::ImageFactory::open(targetPath.toUtf8().toStdString());
    xmp_image->readMetadata();
    const Exiv2::XmpData &xmpData = xmp_image->xmpData();
    for (auto md = xmpData.begin(); md != xmpData.end(); ++md) {
      xmpEntries.append({QString::fromStdString(md->key()),
                         QString::fromStdString(md->value().toString())});
    }
  } catch (const Exiv2::Error &e) {
    qWarning() << "Exiv2 Error in readSrcXmp:" << e.what();
  }

  QMetaObject::invokeMethod(
      this, [this, targetPath, xmpEntries = std::move(xmpEntries)]() {
        if (pathToImage != targetPath)
          return;
        populateMetaTable(ui->xmpTableWidget, "XMP", xmpEntries,
                          RzMetadata::xmpTagDescriptions, true);
      });
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

  rotateMnu = new QMenu(this);

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
  exportMnu = new QMenu(this);
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

void PictureWidget::createAvifExportMenu() {
  exportAvifMnu = new QMenu(this);
  exportAvifMnu->setIcon(QIcon(":/resources/img/icons8-send-file-50.png"));

  auto addExportAction = [this](int size) {
    QAction *act =
        new QAction(QIcon(":/resources/img/icons8-ausgang-48.png"),
                    tr("export to size") + " " + QString::number(size), this);
    connect(act, &QAction::triggered, this,
            [this, size] { PictureWidget::exportSrcImgToAvif(size); });
    exportAvifMnu->addAction(act);
  };

  addExportAction(480);
  addExportAction(640);
  addExportAction(800);
  addExportAction(1024);
  addExportAction(1280);

  exportAvifMnu->addSeparator();

  avif_size_all = new QAction(QIcon(":/resources/img/icons8-send-file-50.png"),
                              tr("export to all sizes"), this);
  connect(avif_size_all, &QAction::triggered, this,
          [this] { PictureWidget::exportSrcImgToAvifThread(); });
  exportAvifMnu->addAction(avif_size_all);

  exportAvifMnu->addSeparator();

  avif_oversizeAct = new QAction(tr("increase too small picture"), this);
  avif_oversizeAct->setCheckable(true);
  avif_oversizeAct->setChecked(true);
  exportAvifMnu->addAction(avif_oversizeAct);

  avif_overwriteAvifAct = new QAction(tr("overwrite existing AVIF"), this);
  avif_overwriteAvifAct->setCheckable(true);
  avif_overwriteAvifAct->setChecked(true);
  exportAvifMnu->addAction(avif_overwriteAvifAct);

  avif_watermarkAvifAct = new QAction(tr("watermark AVIF"), this);
  avif_watermarkAvifAct->setCheckable(true);
  avif_watermarkAvifAct->setChecked(true);
  exportAvifMnu->addAction(avif_watermarkAvifAct);

  ui->exportSrcToAvif_Btn->setMenu(exportAvifMnu);
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
  Photo photo(pathToImage);
  photo.rotateImage(val);

  picture = QPixmap(pathToImage);
  resizePicture();

  // Signal emitten für Thumbnail Refresh
  emit imageChanged(pathToImage);
}

void PictureWidget::exportSrcImgToWebpThread() {
  Photo phot(pathToImage);

  if (webp_oversizeAct->isChecked())
    phot.setOversizeSmallerPicture(true);
  if (webp_overwriteWebpAct->isChecked())
    phot.setOverwriteExistingWebp(true);
  if (webp_watermarkWebpAct->isChecked())
    phot.setWatermarkWebp(true);

  const QString targetPath = pathToImage;
  (void)QtConcurrent::run([this, phot, targetPath]() mutable {
    bool oknok = phot.convertImages(75);
    QMetaObject::invokeMethod(
        this,
        [this, targetPath, oknok]() {
          QMessageBox msgBox(this);
          msgBox.setWindowTitle(oknok ? tr("Success") : tr("Error"));
          msgBox.setIcon(oknok ? QMessageBox::Information
                               : QMessageBox::Warning);
          msgBox.setText(tr("WebP export to subfolder WebP") +
                         (oknok ? " " + tr("successful") : " " + tr("failed")));
          msgBox.setInformativeText("<i>" + targetPath.toHtmlEscaped() +
                                    "</i>");
          msgBox.setFixedWidth(900);
          msgBox.exec();
        },
        Qt::QueuedConnection);
  });
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

void PictureWidget::exportSrcImgToAvifThread() {
  Photo phot(pathToImage);

  if (avif_oversizeAct->isChecked())
    phot.setOversizeSmallerPicture(true);
  if (avif_overwriteAvifAct->isChecked())
    phot.setOverwriteExistingWebp(true);
  if (avif_watermarkAvifAct->isChecked())
    phot.setWatermarkWebp(true);

  const QString targetPath = pathToImage;
  (void)QtConcurrent::run([this, phot, targetPath]() mutable {
    bool oknok = phot.convertImagesAvif(75);
    QMetaObject::invokeMethod(
        this,
        [this, targetPath, oknok]() {
          QMessageBox msgBox(this);
          msgBox.setWindowTitle(oknok ? tr("Success") : tr("Error"));
          msgBox.setIcon(oknok ? QMessageBox::Information
                               : QMessageBox::Warning);
          msgBox.setText(tr("AVIF export to subfolder AVIF") +
                         (oknok ? " " + tr("successful") : " " + tr("failed")));
          msgBox.setInformativeText("<i>" + targetPath.toHtmlEscaped() +
                                    "</i>");
          msgBox.setFixedWidth(900);
          msgBox.exec();
        },
        Qt::QueuedConnection);
  });
}

void PictureWidget::exportSrcImgToAvif(int size) {
  Photo photo(pathToImage);

  if (avif_oversizeAct->isChecked())
    photo.setOversizeSmallerPicture(true);
  if (avif_overwriteAvifAct->isChecked())
    photo.setOverwriteExistingWebp(true);
  if (avif_watermarkAvifAct->isChecked())
    photo.setWatermarkWebp(true);

  bool result = photo.convertImageAvif(size, 75);

  QMessageBox msgBox(this);
  msgBox.setWindowTitle(result ? tr("Success") : tr("Error"));
  msgBox.setIcon(result ? QMessageBox::Information : QMessageBox::Warning);
  msgBox.setText(tr("AVIF export") +
                 (result ? " " + tr("successful") : " " + tr("failed")));
  msgBox.setInformativeText("<i>" + pathToImage.toHtmlEscaped() + "</i>");
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