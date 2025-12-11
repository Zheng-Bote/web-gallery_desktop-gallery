/**
 * @file rz_photo.hpp
 * @author ZHENG Robert
 * @brief Wrapper class for Exiv2 and Image Operations
 */
#pragma once

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QString>
#include <QStringList>
#include <exiv2/exiv2.hpp>
#include <tuple>

#include "rz_metadata.hpp"

struct ImageConvertStruct {
  QString fileName{""};
  QString fileBasename{""};
  QString fileSuffix{""};
  QString fileAbsolutePath{""};
  QString newFolder{"WebP"};
  QString newSuffix{"webp"};
  QList<int> webpSizes = {480, 680, 800, 1024, 1280};
};

class Photo {
public:
  // --- ZENTRALE DEFINITIONEN ---
  inline static const QStringList allSupportedTypes = {
      "jpg", "jpeg", "png", "webp", "tiff", "tif", "bmp"};

  inline static const QStringList metaSupportedTypes = {"jpg",  "jpeg", "png",
                                                        "webp", "tiff", "tif"};

  Photo();
  explicit Photo(const QString &imageInput);
  ~Photo();

  // --- Core Functions ---
  void readExif();

  const bool convertImage(const int &targetSize, const int &quality = 75);
  const bool convertImages(const int &quality = 75);
  bool rotateImage(const int &turn);
  const std::tuple<bool, QString> renameImageToTimestamp();

  // --- Metadata Operations ---
  bool writeXmp(const QString &key, const QString &value);
  bool writeExif(const QString &key, const QString &value);
  bool writeIptc(const QString &key, const QString &value);

  // NEU: Bulk Write Support
  bool writeAllMetadata(const RzMetadata::DefaultMetaStruct &metaData);
  QString getRawTagValue(const QString &key);

  QString getCopyright();
  bool writeToAllCopyrightOwner(const QString &owner);

  // --- GPS Utils ---
  struct ExifGpsStruct {
    QString GPSTag{""};
    QString GPSLatitudeRef{""};
    QString GPSLatitude{""};
    QString GPSLongitudeRef{""};
    QString GPSLongitude{""};
    QString GPSAltitudeRef{""};
    QString GPSAltitude{""};
    QString GPSTimeStamp{""};
    QString GPSMapDatum{""};
    QString GPSDateStamp{""};
    QString DateTimeOriginal{""};
  };
  ExifGpsStruct getGpsData() const;
  QString getGpsString();
  QString getGpsDecimalString();
  QPointF getGpsLatLon();

  // --- Settings & Getter ---
  void setOversizeSmallerPicture(bool enable) {
    oversizeSmallerPicture_bool = enable;
  }
  void setOverwriteExistingWebp(bool enable) {
    overwriteExitingWebp_bool = enable;
  }
  void setWatermarkWebp(bool enable) { watermarkWebp_bool = enable; }
  void setRenameToTimestamp(bool enable) { renameToTimestamp_bool = enable; }

  QString getSuffix() const { return imgStruct.fileSuffix; }
  QString getImgNewTimestampName() const;

  // --- SUPPORT CHECKS (JETZT PUBLIC) ---
  bool isValidImageType();
  bool hasMetadataSupport();

  // Static Helpers
  static QList<QString> srcPics(const QString &srcPath);
  static QList<QString> srcPicsRecursive(const QString &srcPath);

private:
  ImageConvertStruct imgStruct;
  void setImageStruct(const QString &imageInput);

  bool oversizeSmallerPicture_bool{false};
  bool overwriteExitingWebp_bool{false};
  bool watermarkWebp_bool{true};
  bool renameToTimestamp_bool{false};

  bool createWebpPath();
  // Ermittelt Zeitstempel (aus Exif oder Datei) ohne umzubenennen
  QString determineTimestamp();
};