/**
 * @file rz_photo.cpp
 * @author ZHENG Robert
 * @brief Implementation of Photo operations
 */
#include "rz_photo.hpp"
#include <QBuffer>
#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QDirIterator>
#include <QFont>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QPointF>
#include <QRegularExpression>

#include <cmath>

#define CATCH_EXIV2                                                            \
  catch (Exiv2::Error & e) {                                                   \
    qWarning() << "Exiv2 Error:" << e.what();                                  \
    return false;                                                              \
  }

Photo::Photo() {}

Photo::Photo(const QString &imageInput) { setImageStruct(imageInput); }

Photo::~Photo() {}

void Photo::setImageStruct(const QString &imageInput) {
  QFileInfo fi(imageInput);
  imgStruct.fileAbsolutePath = fi.absoluteFilePath();
  imgStruct.fileName = fi.fileName();
  imgStruct.fileBasename = fi.baseName();
  imgStruct.fileSuffix = fi.suffix();
}

// --- CHECKS ---

bool Photo::isValidImageType() {
  return Photo::allSupportedTypes.contains(imgStruct.fileSuffix,
                                           Qt::CaseInsensitive);
}

bool Photo::hasMetadataSupport() {
  return Photo::metaSupportedTypes.contains(imgStruct.fileSuffix,
                                            Qt::CaseInsensitive);
}

// --- HELPER: TIMESTAMP ERMITTELN ---

QString Photo::determineTimestamp() {
  QDateTime finalDate;

  // 1. Metadaten (wenn unterstützt)
  if (hasMetadataSupport()) {
    try {
      auto image =
          Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
      image->readMetadata();
      Exiv2::ExifData &exifData = image->exifData();

      if (!exifData.empty()) {
        std::string dateStr;
        if (exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal")) !=
            exifData.end()) {
          dateStr = exifData["Exif.Photo.DateTimeOriginal"].toString();
        } else if (exifData.findKey(Exiv2::ExifKey("Exif.Image.DateTime")) !=
                   exifData.end()) {
          dateStr = exifData["Exif.Image.DateTime"].toString();
        }

        if (!dateStr.empty() && dateStr.substr(0, 4) != "0000") {
          QString qDateStr = QString::fromStdString(dateStr).trimmed();
          finalDate = QDateTime::fromString(qDateStr, "yyyy:MM:dd HH:mm:ss");
          if (!finalDate.isValid()) {
            finalDate = QDateTime::fromString(qDateStr, "yyyy-MM-dd HH:mm:ss");
          }
        }
      }
    } catch (...) {
    }
  }

  // 2. Fallback Dateisystem
  if (!finalDate.isValid()) {
    QFileInfo fi(imgStruct.fileAbsolutePath);
    finalDate = fi.birthTime();
    if (!finalDate.isValid())
      finalDate = fi.lastModified();
  }

  if (finalDate.isValid()) {
    return finalDate.toString("yyyy-MM-dd_HHmmss");
  }
  return "";
}

// --- WEBP KONVERTIERUNG ---

const bool Photo::convertImages(const int &quality) {
  bool allSuccess = true;
  if (!convertImage(0, quality)) {
    allSuccess = false;
    qWarning() << "WebP Export failed for Original Size";
  }
  for (int size : imgStruct.webpSizes) {
    if (!convertImage(size, quality)) {
      allSuccess = false;
      qWarning() << "WebP Export failed for size:" << size;
    }
  }
  return allSuccess;
}

const bool Photo::convertImage(const int &targetSize, const int &quality) {
  if (!isValidImageType())
    return false;
  if (!createWebpPath())
    return false;

  // Namens-Logik (Timestamp Rename)
  QString targetBasename = imgStruct.fileBasename;
  if (renameToTimestamp_bool) {
    QString ts = determineTimestamp();
    if (!ts.isEmpty()) {
      targetBasename = ts;
    }
  }

  QString sizeSuffix =
      (targetSize > 0) ? "_" + QString::number(targetSize) : "";
  QString targetPath = imgStruct.fileAbsolutePath.left(
                           imgStruct.fileAbsolutePath.lastIndexOf('/')) +
                       "/" + imgStruct.newFolder + "/" + targetBasename +
                       sizeSuffix + "." + imgStruct.newSuffix;

  QFile targetFile(targetPath);
  if (targetFile.exists() && !overwriteExitingWebp_bool)
    return true;

  QImageReader reader(imgStruct.fileAbsolutePath);
  if (!reader.canRead())
    return false;

  if (targetSize > 0) {
    QSize originalSize = reader.size();
    if (originalSize.width() > targetSize ||
        originalSize.height() > targetSize) {
      reader.setScaledSize(
          originalSize.scaled(targetSize, targetSize, Qt::KeepAspectRatio));
    }
  }

  QImage img = reader.read();
  if (img.isNull())
    return false;

  if (targetSize > 0 && oversizeSmallerPicture_bool) {
    if (img.width() < targetSize && img.height() < targetSize) {
      img = img.scaled(targetSize, targetSize, Qt::KeepAspectRatio,
                       Qt::SmoothTransformation);
    }
  }

  if (watermarkWebp_bool) {
    QPainter p(&img);
    QFont f = p.font();
    f.setPixelSize(qMax(20, img.height() / 30));
    f.setBold(true);
    p.setFont(f);

    QString text = "reduced copy";
    int padding = 20;
    QRect textRect = img.rect().adjusted(padding, padding, -padding, -padding);

    p.setPen(QColor(0, 0, 0, 160));
    p.drawText(textRect.translated(2, 2), Qt::AlignTop | Qt::AlignLeft, text);

    p.setPen(QColor(255, 255, 255, 220));
    p.drawText(textRect, Qt::AlignTop | Qt::AlignLeft, text);
  }

  QImageWriter writer(targetPath, "webp");
  writer.setQuality(quality);
  return writer.write(img);
}

bool Photo::createWebpPath() {
  QDir dir(QFileInfo(imgStruct.fileAbsolutePath).absolutePath());
  if (!dir.exists(imgStruct.newFolder)) {
    return dir.mkdir(imgStruct.newFolder);
  }
  return true;
}

// --- RENAME SOURCE FILE ---

const std::tuple<bool, QString> Photo::renameImageToTimestamp() {
  if (!isValidImageType()) {
    qWarning() << "[Rename] Typ nicht unterstützt:" << imgStruct.fileSuffix;
    return {false, ""};
  }

  QString newName = determineTimestamp();

  if (newName.isEmpty()) {
    qWarning() << "[Rename] Kein Datum gefunden:" << imgStruct.fileName;
    return {false, ""};
  }

  QString dirPath = QFileInfo(imgStruct.fileAbsolutePath).absolutePath();
  QString newPath = dirPath + "/" + newName + "." + imgStruct.fileSuffix;

  int counter = 1;
  while (QFile::exists(newPath)) {
    if (newPath == imgStruct.fileAbsolutePath)
      return {true, newPath};
    newPath = dirPath + "/" + newName + "_" + QString::number(counter++) + "." +
              imgStruct.fileSuffix;
  }

  QFile f(imgStruct.fileAbsolutePath);
  if (f.rename(newPath)) {
    setImageStruct(newPath);
    return {true, newPath};
  }
  return {false, ""};
}

QString Photo::getImgNewTimestampName() const { return ""; }

// --- METADATA OPERATIONS ---

bool Photo::writeXmp(const QString &key, const QString &value) {
  if (!hasMetadataSupport())
    return false;
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();
    Exiv2::XmpData &xmpData = image->xmpData();
    xmpData[key.toStdString()] = value.toStdString();
    image->writeMetadata();
    return true;
  }
  CATCH_EXIV2
}

bool Photo::writeExif(const QString &key, const QString &value) {
  if (!hasMetadataSupport())
    return false;
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    exifData[key.toStdString()] = value.toStdString();
    image->writeMetadata();
    return true;
  }
  CATCH_EXIV2
}

bool Photo::writeIptc(const QString &key, const QString &value) {
  if (!hasMetadataSupport())
    return false;
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();
    Exiv2::IptcData &iptcData = image->iptcData();
    iptcData[key.toStdString()] = value.toStdString();
    image->writeMetadata();
    return true;
  }
  CATCH_EXIV2
}

bool Photo::writeAllMetadata(const RzMetadata::DefaultMetaStruct &metaData) {
  if (!hasMetadataSupport())
    return false;

  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    Exiv2::IptcData &iptcData = image->iptcData();
    Exiv2::XmpData &xmpData = image->xmpData();

    QHashIterator<QString, QString> itExif(metaData.exifDefault);
    while (itExif.hasNext()) {
      itExif.next();
      if (!itExif.value().isEmpty())
        exifData[itExif.key().toStdString()] = itExif.value().toStdString();
    }

    QHashIterator<QString, QString> itIptc(metaData.iptcDefault);
    while (itIptc.hasNext()) {
      itIptc.next();
      if (!itIptc.value().isEmpty())
        iptcData[itIptc.key().toStdString()] = itIptc.value().toStdString();
    }

    QHashIterator<QString, QString> itXmp(metaData.xmpDefault);
    while (itXmp.hasNext()) {
      itXmp.next();
      if (!itXmp.value().isEmpty())
        xmpData[itXmp.key().toStdString()] = itXmp.value().toStdString();
    }

    image->writeMetadata();
    return true;
  }
  CATCH_EXIV2
}

QString Photo::getRawTagValue(const QString &key) {
  if (!hasMetadataSupport())
    return "";
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();

    if (key.startsWith("Exif.")) {
      if (image->exifData().findKey(Exiv2::ExifKey(key.toStdString())) !=
          image->exifData().end())
        return QString::fromStdString(
            image->exifData()[key.toStdString()].toString());
    } else if (key.startsWith("Iptc.")) {
      if (image->iptcData().findKey(Exiv2::IptcKey(key.toStdString())) !=
          image->iptcData().end())
        return QString::fromStdString(
            image->iptcData()[key.toStdString()].toString());
    } else if (key.startsWith("Xmp.")) {
      if (image->xmpData().findKey(Exiv2::XmpKey(key.toStdString())) !=
          image->xmpData().end())
        return QString::fromStdString(
            image->xmpData()[key.toStdString()].toString());
    }
  } catch (...) {
  }
  return "";
}

// FIX: Liefert wieder exakt das, was in der Datei steht. Kein Cleanup.
QString Photo::getCopyright() {
  if (!hasMetadataSupport())
    return "";
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();

    // 1. Priorität: XMP
    Exiv2::XmpData &xmpData = image->xmpData();
    if (xmpData.findKey(Exiv2::XmpKey("Xmp.plus.CopyrightOwner")) !=
        xmpData.end())
      return QString::fromStdString(
          xmpData["Xmp.plus.CopyrightOwner"].toString());
    if (xmpData.findKey(Exiv2::XmpKey("Xmp.dc.rights")) != xmpData.end())
      return QString::fromStdString(xmpData["Xmp.dc.rights"].toString());
    if (xmpData.findKey(Exiv2::XmpKey("Xmp.dc.creator")) != xmpData.end())
      return QString::fromStdString(xmpData["Xmp.dc.creator"].toString());

    // 2. Priorität: IPTC
    Exiv2::IptcData &iptcData = image->iptcData();
    if (iptcData.findKey(Exiv2::IptcKey("Iptc.Application2.Copyright")) !=
        iptcData.end())
      return QString::fromStdString(
          iptcData["Iptc.Application2.Copyright"].toString());
    if (iptcData.findKey(Exiv2::IptcKey("Iptc.Application2.Byline")) !=
        iptcData.end())
      return QString::fromStdString(
          iptcData["Iptc.Application2.Byline"].toString());

    // 3. Priorität: EXIF
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.findKey(Exiv2::ExifKey("Exif.Image.Copyright")) !=
        exifData.end())
      return QString::fromStdString(
          exifData["Exif.Image.Copyright"].toString());
    if (exifData.findKey(Exiv2::ExifKey("Exif.Image.Artist")) != exifData.end())
      return QString::fromStdString(exifData["Exif.Image.Artist"].toString());

  } catch (...) {
  }
  return "";
}

bool Photo::writeToAllCopyrightOwner(const QString &owner) {
  if (!hasMetadataSupport())
    return false;
  bool ok = true;
  ok &= writeXmp("Xmp.plus.CopyrightOwner", owner);
  ok &= writeXmp("Xmp.dc.creator", owner);
  ok &= writeXmp("Xmp.dc.rights",
                 owner); // Normaler Text, User kann © hinzufügen wenn er will

  ok &= writeExif("Exif.Image.Artist", owner);
  ok &= writeExif("Exif.Image.Copyright", owner);

  ok &= writeIptc("Iptc.Application2.Byline", owner);
  ok &= writeIptc("Iptc.Application2.Copyright", owner);

  return ok;
}

// --- GPS UTILS ---

static double convertToDecimal(const Exiv2::ExifData &exifData,
                               const std::string &keyVal,
                               const std::string &keyRef) {
  try {
    auto pos = exifData.findKey(Exiv2::ExifKey(keyVal));
    if (pos == exifData.end())
      return 0.0;

    auto v = pos->getValue();
    if (v->count() != 3)
      return 0.0;

    double deg = v->toFloat(0);
    double min = v->toFloat(1);
    double sec = v->toFloat(2);
    double decimal = deg + (min / 60.0) + (sec / 3600.0);

    auto posRef = exifData.findKey(Exiv2::ExifKey(keyRef));
    if (posRef != exifData.end()) {
      std::string ref = posRef->toString();
      if (ref == "S" || ref == "W")
        decimal *= -1.0;
    }
    return decimal;
  } catch (...) {
    return 0.0;
  }
}

QPointF Photo::getGpsLatLon() {
  if (!hasMetadataSupport())
    return QPointF(0.0, 0.0);
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    if (exifData.empty())
      return QPointF(0.0, 0.0);

    double lat = convertToDecimal(exifData, "Exif.GPSInfo.GPSLatitude",
                                  "Exif.GPSInfo.GPSLatitudeRef");
    double lon = convertToDecimal(exifData, "Exif.GPSInfo.GPSLongitude",
                                  "Exif.GPSInfo.GPSLongitudeRef");

    return QPointF(lat, lon);
  } catch (...) {
    return QPointF(0.0, 0.0);
  }
}

Photo::ExifGpsStruct Photo::getGpsData() const {
  ExifGpsStruct gps;
  if (!const_cast<Photo *>(this)->hasMetadataSupport())
    return gps;
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    if (exifData.empty())
      return gps;

    auto getVal = [&](const char *key) {
      if (exifData.findKey(Exiv2::ExifKey(key)) != exifData.end())
        return QString::fromStdString(exifData[key].toString());
      return QString();
    };

    gps.GPSLatitude = getVal("Exif.GPSInfo.GPSLatitude");
    gps.GPSLatitudeRef = getVal("Exif.GPSInfo.GPSLatitudeRef");
    gps.GPSLongitude = getVal("Exif.GPSInfo.GPSLongitude");
    gps.GPSLongitudeRef = getVal("Exif.GPSInfo.GPSLongitudeRef");
    gps.GPSAltitude = getVal("Exif.GPSInfo.GPSAltitude");
  } catch (...) {
  }
  return gps;
}

QString Photo::getGpsString() {
  if (!hasMetadataSupport())
    return "";
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    Exiv2::ExifKey keyLat("Exif.GPSInfo.GPSLatitude");
    if (exifData.findKey(keyLat) == exifData.end())
      return "";

    return QString::fromStdString(
        exifData["Exif.GPSInfo.GPSLatitude"].toString() + " " +
        exifData["Exif.GPSInfo.GPSLatitudeRef"].toString() + ", " +
        exifData["Exif.GPSInfo.GPSLongitude"].toString() + " " +
        exifData["Exif.GPSInfo.GPSLongitudeRef"].toString());
  } catch (...) {
  }
  return "";
}

QString Photo::getGpsDecimalString() {
  if (!hasMetadataSupport())
    return "";
  try {
    auto image =
        Exiv2::ImageFactory::open(imgStruct.fileAbsolutePath.toStdString());
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty())
      return "";

    double lat = convertToDecimal(exifData, "Exif.GPSInfo.GPSLatitude",
                                  "Exif.GPSInfo.GPSLatitudeRef");
    double lon = convertToDecimal(exifData, "Exif.GPSInfo.GPSLongitude",
                                  "Exif.GPSInfo.GPSLongitudeRef");

    if (lat == 0.0 && lon == 0.0)
      return "";
    return QString::number(lat, 'f', 5) + ", " + QString::number(lon, 'f', 5);
  } catch (...) {
    return "";
  }
}

bool Photo::rotateImage(const int &turn) {
  QImage img(imgStruct.fileAbsolutePath);
  if (img.isNull())
    return false;

  QTransform trans;
  trans.rotate(turn);
  img = img.transformed(trans, Qt::SmoothTransformation);

  return img.save(imgStruct.fileAbsolutePath);
}

// --- STATIC HELPERS ---

static bool isImageExtensionValid(const QFileInfo &fi) {
  return Photo::allSupportedTypes.contains(fi.suffix(), Qt::CaseInsensitive);
}

QList<QString> Photo::srcPics(const QString &srcPath) {
  QDir dir(srcPath);
  dir.setFilter(QDir::Files);
  QList<QString> list;
  for (const auto &fi : dir.entryInfoList()) {
    if (isImageExtensionValid(fi))
      list << fi.absoluteFilePath();
  }
  return list;
}

QList<QString> Photo::srcPicsRecursive(const QString &srcPath) {
  QList<QString> list;
  QDirIterator it(srcPath, QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    if (isImageExtensionValid(it.fileInfo()))
      list << it.filePath();
  }
  return list;
}

void Photo::readExif() {}