/**
 * @file picture_widget.cpp
 * @author ZHENG Robert (robert.hase-zheng.net)
 * @brief Main class for the picture detail widget
 * @version 0.3
 * @date 2025-03-01
 *
 * @copyright Copyright (c) 2025 ZHENG Robert
 *
 */
#include "picture_widget.h"
#include "ui_picture_widget.h"
#include "rz_metadata.hpp" 

#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QCloseEvent>

// Exiv2 und QtConcurrent Includes
#include <exiv2/exiv2.hpp>
#include <QFuture>
#include <QFutureWatcher>
#include <QThread>
#include <QtConcurrent>

PictureWidget::PictureWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::PictureWidget)
{
    ui->setupUi(this);

    // Initialisierung von Exiv2
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
    
    ui->closeBtn->setIcon(QIcon(":/resources/img/icons8-arrow-pointing-left-50.png"));
    ui->exportSrcToWebp_Btn->setIcon(QIcon(":/resources/img/icons8-send-file-50.png"));
    
    picture = nullptr;

    createRotateMenu();
    createExportMenu();
}

PictureWidget::~PictureWidget()
{
    delete ui;

    if (picture) {
        delete picture;
        picture = nullptr;
    }
}

void PictureWidget::setImage(QString pathToFile)
{
    pathToImage = pathToFile;
    QFile srcFile(pathToFile);
    QFileInfo fileInfo(srcFile.fileName());

    if (picture) {
        delete picture;
    }
    picture = new QPixmap(pathToImage);

    QString pictureData = fileInfo.fileName() + " \n(" + QString::number(picture->width()) + "x" + QString::number(picture->height()) + " " + QString::number(fileInfo.size() / 1024) + " KiB)";
    ui->filename_label->setText(pictureData);
    ui->filename_label->setToolTip(pathToFile);

    ui->picture_label->setPixmap(
        picture->scaled(ui->picture_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QFuture<void> futureExif = QtConcurrent::run(&PictureWidget::readSrcExif, this);
    QFuture<void> futureIptc = QtConcurrent::run(&PictureWidget::readSrcIptc, this);
    QFuture<void> futureXmp = QtConcurrent::run(&PictureWidget::readSrcXmp, this);

    ui->tabWidget->adjustSize();
    ui->tabWidget->setCurrentWidget(0);
}

void PictureWidget::on_closeBtn_clicked()
{
    this->close();
}

void PictureWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizePicture();
}

void PictureWidget::closeEvent(QCloseEvent *event)
{
    if (dataModified_bool)
    {
        QMessageBox::StandardButton response;

        response = QMessageBox::question(this,
                                         tr("Confirmation"),
                                         tr("Are you sure you want to exit without saving data?"),
                                         QMessageBox::Yes | QMessageBox::No);

        if (response == QMessageBox::Yes)
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->accept();
    }
}

void PictureWidget::resizePicture()
{
    if (picture && !picture->isNull()) {
        ui->picture_label->setPixmap(
            picture->scaled(ui->picture_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

bool PictureWidget::checkValidMetaImg()
{
    bool ret{false};
    Photo *photo = new Photo(pathToImage);
    QString photoExtension = photo->getSuffix().toLower();

    if (validMetaImageTypes.contains(photoExtension))
    {
        ret = true;
    }
    
    delete photo;
    return ret;
}

void PictureWidget::disableMetaTabs(int tab)
{
    ui->tabWidget->setTabVisible(tab, false);
}

const void PictureWidget::readSrcExif()
{
    if (!checkValidMetaImg())
    {
        disableMetaTabs(1);
        return;
    }

    QFile file(pathToImage);
    auto exif_image = Exiv2::ImageFactory::open(file.fileName().toUtf8().toStdString());

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

    if (!exifData.empty())
    {
        auto end = exifData.end();
        for (auto md = exifData.begin(); md != end; ++md)
        {
            ui->exifTableWidget->insertRow(ui->exifTableWidget->rowCount());
            ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1,
                                         0,
                                         new QTableWidgetItem(md->key().c_str()));

            QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
            tlbCol2val->setText(md->value().toString().c_str());
            ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1, 1, tlbCol2val);

            if (RzMetadata::exifTagDescriptions.contains(md->key().c_str()))
            {
                QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
                tlbCol3val->setText(RzMetadata::exifTagDescriptions.value(md->key().c_str(), ""));
                ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1, 2, tlbCol3val);
            }
        }
    }

    for (auto i = RzMetadata::exifTagDescriptions.cbegin(), end = RzMetadata::exifTagDescriptions.cend(); i != end; ++i)
    {
        markExif(i.key());
    }

    file.close();
}

void PictureWidget::markExif(QString searchFor)
{
    QList<QTableWidgetItem *> LTempTable = ui->exifTableWidget->findItems(searchFor, Qt::MatchEndsWith);

    if (LTempTable.isEmpty())
    {
        ui->exifTableWidget->insertRow(ui->exifTableWidget->rowCount());
        ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1,
                                     0,
                                     new QTableWidgetItem(searchFor));

        QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
        tlbCol2val->setText("");
        ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1, 1, tlbCol2val);

        if (RzMetadata::exifTagDescriptions.contains(searchFor))
        {
            QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
            tlbCol3val->setText(RzMetadata::exifTagDescriptions.value(searchFor, ""));
            ui->exifTableWidget->setItem(ui->exifTableWidget->rowCount() - 1, 2, tlbCol3val);
        }
    }

    QTableWidgetItem *rowPtr = new QTableWidgetItem();
    LTempTable = ui->exifTableWidget->findItems(searchFor, Qt::MatchEndsWith);
    foreach (rowPtr, LTempTable)
    {
        int rowNumber = rowPtr->row();
        if(ui->exifTableWidget->item(rowNumber, 0))
             ui->exifTableWidget->item(rowNumber, 0)->setForeground(Qt::red);
    }
}

const void PictureWidget::readSrcIptc()
{
    if (!checkValidMetaImg())
    {
        disableMetaTabs(2);
        return;
    }
    QFile file(pathToImage);
    auto iptc_image = Exiv2::ImageFactory::open(file.fileName().toUtf8().toStdString());

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

    if (!iptcData.empty())
    {
        auto end = iptcData.end();
        for (auto md = iptcData.begin(); md != end; ++md)
        {
            ui->iptcTableWidget->insertRow(ui->iptcTableWidget->rowCount());
            ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1,
                                         0,
                                         new QTableWidgetItem(md->key().c_str()));

            QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
            tlbCol2val->setText(md->value().toString().c_str());
            ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1, 1, tlbCol2val);

            if (RzMetadata::iptcTagDescriptions.contains(md->key().c_str()))
            {
                QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
                tlbCol3val->setText(RzMetadata::iptcTagDescriptions.value(md->key().c_str(), ""));
                ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1, 2, tlbCol3val);
            }
        }
    }

    for (auto i = RzMetadata::iptcTagDescriptions.cbegin(), end = RzMetadata::iptcTagDescriptions.cend(); i != end; ++i)
    {
        markIptc(i.key());
    }

    file.close();
}

const void PictureWidget::readSrcXmp()
{
    if (!checkValidMetaImg())
    {
        disableMetaTabs(3);
        return;
    }
    QFile file(pathToImage);
    auto xmp_image = Exiv2::ImageFactory::open(file.fileName().toUtf8().toStdString());

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

    if (!xmpData.empty())
    {
        auto end = xmpData.end();
        for (auto md = xmpData.begin(); md != end; ++md)
        {
            ui->xmpTableWidget->insertRow(ui->xmpTableWidget->rowCount());
            ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1,
                                        0,
                                        new QTableWidgetItem(md->key().c_str()));

            QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
            tlbCol2val->setText(md->value().toString().c_str());
            ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1, 1, tlbCol2val);

            if (RzMetadata::xmpTagDescriptions.contains(md->key().c_str()))
            {
                QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
                tlbCol3val->setText(RzMetadata::xmpTagDescriptions.value(md->key().c_str(), ""));
                ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1, 2, tlbCol3val);
            }
        }
    }

    for (auto i = RzMetadata::xmpTagDescriptions.cbegin(), end = RzMetadata::xmpTagDescriptions.cend(); i != end; ++i)
    {
        markXmp(i.key());
    }

    file.close();

    ui->xmpTableWidget->resizeColumnsToContents();
    ui->xmpTableWidget->resizeRowsToContents();
}

void PictureWidget::markXmp(QString searchFor)
{
    QList<QTableWidgetItem *> LTempTable = ui->xmpTableWidget->findItems(searchFor, Qt::MatchEndsWith);

    if (LTempTable.isEmpty())
    {
        ui->xmpTableWidget->insertRow(ui->xmpTableWidget->rowCount());
        ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1,
                                    0,
                                    new QTableWidgetItem(searchFor));

        QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
        tlbCol2val->setText("");
        ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1, 1, tlbCol2val);

        if (RzMetadata::xmpTagDescriptions.contains(searchFor))
        {
            QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
            tlbCol3val->setText(RzMetadata::xmpTagDescriptions.value(searchFor, ""));
            ui->xmpTableWidget->setItem(ui->xmpTableWidget->rowCount() - 1, 2, tlbCol3val);
        }
    }

    QTableWidgetItem *rowPtr = new QTableWidgetItem();
    LTempTable = ui->xmpTableWidget->findItems(searchFor, Qt::MatchEndsWith);
    foreach (rowPtr, LTempTable)
    {
        int rowNumber = rowPtr->row();
        if(ui->xmpTableWidget->item(rowNumber, 0))
             ui->xmpTableWidget->item(rowNumber, 0)->setForeground(Qt::red);

        if (ui->xmpTableWidget->item(rowNumber, 1) && ui->xmpTableWidget->item(rowNumber, 1)->text().length() < 1)
        {
            ui->xmpTableWidget->item(rowNumber, 1)->setBackground(Qt::red);
            ui->xmpTableWidget->item(rowNumber, 1)->setForeground(Qt::black);
        }
    }
}

void PictureWidget::mapMetaData(const QString &pathToFile,
                                const QString &key,
                                const QString &_value,
                                const QString &source)
{
    QString targetKey{""};
    QString value = _value;
    Photo photo(pathToFile);

    if (source.compare("exif") == 0)
    {
        if (RzMetadata::exif_to_iptc.contains(key))
        {
            targetKey = RzMetadata::exif_to_iptc[key];
            photo.writeIptc(targetKey, value);
        }
        if (RzMetadata::exif_to_xmp.contains(key))
        {
            targetKey = RzMetadata::exif_to_xmp[key];
            photo.writeXmp(targetKey, value);
        }
    }
}

void PictureWidget::markIptc(QString searchFor)
{
    QList<QTableWidgetItem *> LTempTable = ui->iptcTableWidget->findItems(searchFor, Qt::MatchEndsWith);

    if (LTempTable.isEmpty())
    {
        ui->iptcTableWidget->insertRow(ui->iptcTableWidget->rowCount());
        ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1,
                                     0,
                                     new QTableWidgetItem(searchFor));

        QTableWidgetItem *tlbCol2val = new QTableWidgetItem();
        tlbCol2val->setText("");
        ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1, 1, tlbCol2val);

        if (RzMetadata::iptcTagDescriptions.contains(searchFor))
        {
            QTableWidgetItem *tlbCol3val = new QTableWidgetItem();
            tlbCol3val->setText(RzMetadata::iptcTagDescriptions.value(searchFor, ""));
            ui->iptcTableWidget->setItem(ui->iptcTableWidget->rowCount() - 1, 2, tlbCol3val);
        }
    }

    QTableWidgetItem *rowPtr = new QTableWidgetItem();
    LTempTable = ui->iptcTableWidget->findItems(searchFor, Qt::MatchEndsWith);
    foreach (rowPtr, LTempTable)
    {
        int rowNumber = rowPtr->row();
        if(ui->iptcTableWidget->item(rowNumber, 0))
             ui->iptcTableWidget->item(rowNumber, 0)->setForeground(Qt::red);
    }
}

void PictureWidget::createRotateMenu()
{
    ui->rotateSrcImg_Btn->setIcon(QIcon(":/resources/img/icons8-available-updates-50.png"));

    rotateMnu = new QMenu();

    rotate_90 = new QAction(QIcon(":/resources/img/icons8-rotate-right-50.png"), tr("rotate") + " 90°", this);
    connect(rotate_90, &QAction::triggered, this, [this]{ PictureWidget::rotateSrcImg(90); });
    rotateMnu->addAction(rotate_90);

    rotate_120 = new QAction(QIcon(":/resources/img/icons8-rotate-right-50.png"), tr("rotate") + " 120°", this);
    connect(rotate_120, &QAction::triggered, this, [this]{ PictureWidget::rotateSrcImg(120); });
    rotateMnu->addAction(rotate_120);

    rotateMnu->addSeparator();

    rotate_m90 = new QAction(QIcon(":/resources/img/icons8-rotate-left-50.png"), tr("rotate") + " -90°", this);
    connect(rotate_m90, &QAction::triggered, this, [this]{ PictureWidget::rotateSrcImg(-90); });
    rotateMnu->addAction(rotate_m90);

    rotate_m120 = new QAction(QIcon(":/resources/img/icons8-rotate-left-50.png"), tr("rotate") + " -120°", this);
    connect(rotate_m120, &QAction::triggered, this, [this]{ PictureWidget::rotateSrcImg(-120); });
    rotateMnu->addAction(rotate_m120);

    ui->rotateSrcImg_Btn->setMenu(rotateMnu);
}

void PictureWidget::createExportMenu()
{
    exportMnu = new QMenu();
    exportMnu->setIcon(QIcon(":/resources/img/icons8-send-file-50.png"));

    auto addExportAction = [this](int size) {
        QAction* act = new QAction(QIcon(":/resources/img/icons8-ausgang-48.png"), tr("export to size") + " " + QString::number(size), this);
        connect(act, &QAction::triggered, this, [this, size]{ PictureWidget::exportSrcImgToWebP(size); });
        exportMnu->addAction(act);
    };

    addExportAction(480);
    addExportAction(640);
    addExportAction(800);
    addExportAction(1024);
    addExportAction(1280);

    exportMnu->addSeparator();

    webp_size_all = new QAction(QIcon(":/resources/img/icons8-send-file-50.png"), tr("export to all sizes"), this);
    connect(webp_size_all, &QAction::triggered, this, [this]{ PictureWidget::exportSrcImgToWebpThread(); });
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

void PictureWidget::_on_resizeSmallerImage_Btn_clicked()
{
    resize(width() - 100, height() - 100);
}

void PictureWidget::_on_resizeBiggerImage_Btn_clicked()
{
    resize(width() + 100, height() + 100);
}

void PictureWidget::on_exifTableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    bool ok;
    int row = item->row();
    QString key = ui->exifTableWidget->item(row, 0)->text();

    QString text = QInputDialog::getText(this,
                                         tr("Edit Metadata"),
                                         tr("Please enter the new value for") + " " + key + ":",
                                         QLineEdit::Normal,
                                         ui->exifTableWidget->item(row, 1)->text(),
                                         &ok);
    if (ok && !text.isEmpty())
    {
        ui->exifTableWidget->setItem(row, 1, new QTableWidgetItem(text));
        
        try {
            auto image = Exiv2::ImageFactory::open(pathToImage.toStdString());
            image->readMetadata();
            Exiv2::ExifData &exifData = image->exifData();
            exifData[key.toStdString()] = text.toStdString();
            image->writeMetadata();
            dataModified_bool = true;
        } catch (Exiv2::Error &e) {
            qCritical() << "Exiv2 error: " << e.what();
        }
    }
}

void PictureWidget::on_iptcTableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    bool ok;
    int row = item->row();
    QString key = ui->iptcTableWidget->item(row, 0)->text();

    QString text = QInputDialog::getText(this,
                                         tr("Edit Metadata"),
                                         tr("Please enter the new value for") + " " + key + ":",
                                         QLineEdit::Normal,
                                         ui->iptcTableWidget->item(row, 1)->text(),
                                         &ok);
    if (ok && !text.isEmpty())
    {
        ui->iptcTableWidget->setItem(row, 1, new QTableWidgetItem(text));

        try {
            auto image = Exiv2::ImageFactory::open(pathToImage.toStdString());
            image->readMetadata();
            Exiv2::IptcData &iptcData = image->iptcData();
            iptcData[key.toStdString()] = text.toStdString();
            image->writeMetadata();
            dataModified_bool = true;
        } catch (Exiv2::Error &e) {
            qCritical() << "Exiv2 error: " << e.what();
        }
    }
}

void PictureWidget::on_xmpTableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    bool ok;
    int row = item->row();
    QString key = ui->xmpTableWidget->item(row, 0)->text();

    QString text = QInputDialog::getText(this,
                                         tr("Edit Metadata"),
                                         tr("Please enter the new value for") + " " + key + ":",
                                         QLineEdit::Normal,
                                         ui->xmpTableWidget->item(row, 1)->text(),
                                         &ok);
    if (ok && !text.isEmpty())
    {
        ui->xmpTableWidget->setItem(row, 1, new QTableWidgetItem(text));

        try {
            auto image = Exiv2::ImageFactory::open(pathToImage.toStdString());
            image->readMetadata();
            Exiv2::XmpData &xmpData = image->xmpData();
            xmpData[key.toStdString()] = text.toStdString();
            image->writeMetadata();
            dataModified_bool = true;
        } catch (Exiv2::Error &e) {
            qCritical() << "Exiv2 error: " << e.what();
        }
    }
}

void PictureWidget::rotateSrcImg(int val)
{
    Photo *photo = new Photo(pathToImage);
    photo->rotateImage(val);
    delete photo;

    if (picture) delete picture;
    picture = new QPixmap(pathToImage);
    resizePicture();
    
    // NEU: Signal emitten, damit Thumbnail aktualisiert wird
    emit imageChanged(pathToImage);
}

void PictureWidget::exportSrcImgToWebpThread()
{
    bool oknok{false};
    Photo phot(pathToImage);

    if (webp_oversizeAct->isChecked()) phot.setOversizeSmallerPicture(true);
    if (webp_overwriteWebpAct->isChecked()) phot.setOverwriteExistingWebp(true);
    if (webp_watermarkWebpAct->isChecked()) phot.setWatermarkWebp(true);

    QFuture<bool> future = QtConcurrent::run(&Photo::convertImages, phot, 75);
    oknok = future.result();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(oknok ? tr("Success") : tr("Error"));
    msgBox.setIcon(oknok ? QMessageBox::Information : QMessageBox::Warning);
    msgBox.setText(tr("WebP export to subfolder WebP") + (oknok ? " " + tr("successful") : " " + tr("failed")));
    msgBox.setInformativeText("<i>" + pathToImage + "</i>");
    msgBox.setFixedWidth(900);
    msgBox.exec();
}

void PictureWidget::exportSrcImgToWebP(int size)
{
    Photo photo(pathToImage);

    if (webp_oversizeAct->isChecked()) photo.setOversizeSmallerPicture(true);
    if (webp_overwriteWebpAct->isChecked()) photo.setOverwriteExistingWebp(true);
    if (webp_watermarkWebpAct->isChecked()) photo.setWatermarkWebp(true);

    bool result = photo.convertImage(size);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(result ? tr("Success") : tr("Error"));
    msgBox.setIcon(result ? QMessageBox::Information : QMessageBox::Warning);
    msgBox.setText(tr("WebP export") + (result ? " " + tr("successful") : " " + tr("failed")));
    msgBox.setInformativeText("<i>" + pathToImage + "</i>");
    msgBox.setFixedWidth(900);
    msgBox.exec();
}

void PictureWidget::on_tabWidget_tabBarClicked(int index)
{
    ui->exifTableWidget->resizeColumnsToContents();
    ui->iptcTableWidget->resizeColumnsToContents();
    ui->xmpTableWidget->resizeColumnsToContents();
}

void PictureWidget::on_saveBtn_clicked()
{
    // 1. Check: Unterstützt das Format Metadaten? (z.B. BMP nicht)
    Photo photoCheck(pathToImage);
    if (!photoCheck.hasMetadataSupport()) {
        QMessageBox::warning(this, tr("Save Error"), 
            tr("This file format (%1) does not support metadata.").arg(photoCheck.getSuffix()));
        return;
    }

    // Cursor auf Warten setzen
    QApplication::setOverrideCursor(Qt::WaitCursor);

    try {
        // 2. Bild öffnen
        auto image = Exiv2::ImageFactory::open(pathToImage.toStdString());
        image->readMetadata();

        Exiv2::ExifData &exifData = image->exifData();
        Exiv2::IptcData &iptcData = image->iptcData();
        Exiv2::XmpData &xmpData = image->xmpData();

        // --- Helper Lambda zum Speichern einer Tabelle ---
        auto saveTableToData = [](QTableWidget* table, auto& metaContainer) {
            for (int i = 0; i < table->rowCount(); ++i) {
                QString key = table->item(i, 0)->text();
                
                // Value holen (falls Item existiert)
                QString val = "";
                if (table->item(i, 1)) {
                    val = table->item(i, 1)->text();
                }

                // In Container schreiben (nur wenn Key gültig ist)
                if (!key.isEmpty()) {
                    try {
                        // Wenn Wert leer ist -> Tag löschen? 
                        // Strategie hier: Leeren String schreiben oder löschen.
                        // Exiv2 löscht oft automatisch bei Zuweisung von "", 
                        // aber sicherer ist es, den Wert einfach zu setzen.
                        if (val.isEmpty()) {
                            // Option A: Löschen
                            // auto pos = metaContainer.findKey(Exiv2::ExifKey(key.toStdString()));
                            // if (pos != metaContainer.end()) metaContainer.erase(pos);
                            
                            // Option B: Leeren String setzen (bevorzugt für Editor, damit Feld bleibt)
                            metaContainer[key.toStdString()] = "";
                        } else {
                            metaContainer[key.toStdString()] = val.toStdString();
                        }
                    } catch (Exiv2::Error& e) {
                        qWarning() << "Error writing key" << key << ":" << e.what();
                    }
                }
            }
        };

        // 3. Alle Tabellen abarbeiten
        saveTableToData(ui->exifTableWidget, exifData);
        saveTableToData(ui->iptcTableWidget, iptcData);
        saveTableToData(ui->xmpTableWidget, xmpData);

        // 4. Speichern
        image->writeMetadata();

        // 5. Status zurücksetzen
        dataModified_bool = false;
        
        QApplication::restoreOverrideCursor();
        
        // 6. Signal für MainWindow (Thumbnail Refresh)
        emit imageChanged(pathToImage);

        // Optional: Kurzes Feedback in Statuszeile oder via Messagebox
        // QMessageBox::information(this, tr("Saved"), tr("Metadata saved successfully."));

    } catch (Exiv2::Error& e) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Save Error"), 
                              tr("Failed to save metadata:\n%1").arg(e.what()));
    } catch (...) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Save Error"), tr("Unknown error occurred while saving."));
    }
}
