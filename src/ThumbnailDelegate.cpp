#include "ThumbnailDelegate.hpp"
#include <QAbstractItemModel>
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QThreadPool> // Fix: Warning in previous versions

ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
  // Cache auf ca. 300 MB erhöhen
  QPixmapCache::setCacheLimit(300 * 1024);
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &,
                                  const QModelIndex &) const {
  return QSize(250, 250);
}

void ThumbnailDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
  QModelIndex pathIndex = index.model()->index(index.row(), 1);
  QString filePath = pathIndex.data().toString();
  QString cacheKey = "thumb_" + filePath;

  painter->save();
  painter->setClipRect(option.rect);

  if (option.state & QStyle::State_Selected) {
    painter->fillRect(option.rect, option.palette.highlight());
  } else {
    painter->fillRect(option.rect, option.palette.base());
  }

  QPixmap pixmap;
  if (QPixmapCache::find(cacheKey, &pixmap)) {
    QRect rect = option.rect;
    rect.setBottom(rect.bottom() - 30);

    QSize pSize = pixmap.size();
    pSize.scale(rect.size(), Qt::KeepAspectRatio);

    int x = rect.x() + (rect.width() - pSize.width()) / 2;
    int y = rect.y() + (rect.height() - pSize.height()) / 2;

    painter->drawPixmap(x, y, pSize.width(), pSize.height(), pixmap);
  } else {
    QRect placeholderRect = option.rect.adjusted(10, 10, -10, -40);
    painter->setPen(Qt::lightGray);
    painter->drawRect(placeholderRect);

    QFont f = painter->font();
    f.setPointSize(8);
    painter->setFont(f);
    painter->drawText(option.rect.adjusted(0, -20, 0, 0), Qt::AlignCenter,
                      "Lade...");

    QWidget *viewport = const_cast<QWidget *>(option.widget);

    QThreadPool::globalInstance()->start([filePath, cacheKey, viewport]() {
      QFileInfo fi(filePath);
      if (!fi.exists())
        return;

      QImageReader reader(filePath);
      if (!reader.canRead())
        return;

      QSize originalSize = reader.size();
      QSize targetSize(300, 300);

      if (originalSize.isValid() &&
          (originalSize.width() > targetSize.width() ||
           originalSize.height() > targetSize.height())) {
        reader.setScaledSize(
            originalSize.scaled(targetSize, Qt::KeepAspectRatio));
      }

      QImage img = reader.read();
      if (img.isNull())
        return;

      if (viewport) {
        QMetaObject::invokeMethod(viewport, [viewport, cacheKey, img]() {
          QPixmap pm = QPixmap::fromImage(img);
          QPixmapCache::insert(cacheKey, pm);
          viewport->update();
        });
      }
    });
  }

  const int COL_COPYRIGHT = 8;
  const int COL_GPS = 9;

  int textY = option.rect.bottom() - 35;

  QFont overlayFont = painter->font();
  overlayFont.setPixelSize(10);
  overlayFont.setBold(true);
  painter->setFont(overlayFont);

  // Copyright (Rot) - Mit smarter Anzeige
  if (m_showCopyright) {
    QModelIndex idx = index.model()->index(index.row(), COL_COPYRIGHT);
    QString cp = idx.data().toString();
    if (!cp.isEmpty()) {
      painter->setPen(Qt::red);
      QRect rect = option.rect;
      rect.setBottom(textY);
      rect.setTop(textY - 15);

      // LOGIK FIX: Nur "© " hinzufügen, wenn es nicht schon da ist.
      // Erlaubt dem User, "© Name", "Copyright Name" oder nur "Name" zu
      // verwenden.
      QString displayText = cp;
      if (!displayText.startsWith(QChar(0x00A9)) && // © Symbol
          !displayText.startsWith("(c)", Qt::CaseInsensitive) &&
          !displayText.startsWith("Copyright", Qt::CaseInsensitive)) {
        displayText = QString("© ") + displayText;
      }

      painter->drawText(rect, Qt::AlignCenter, displayText);
      textY -= 15;
    }
  }

  // GPS (Dunkelgrün)
  if (m_showGps) {
    QModelIndex idx = index.model()->index(index.row(), COL_GPS);
    QString gps = idx.data().toString();
    if (!gps.isEmpty()) {
      painter->setPen(Qt::darkGreen);
      QRect rect = option.rect;
      rect.setBottom(textY);
      rect.setTop(textY - 15);
      painter->drawText(rect, Qt::AlignCenter, gps);
    }
  }

  QRect textRect = option.rect;
  textRect.setTop(textRect.bottom() - 30);

  painter->setFont(option.font);
  painter->setPen((option.state & QStyle::State_Selected)
                      ? option.palette.highlightedText().color()
                      : option.palette.text().color());

  QString text = option.fontMetrics.elidedText(
      index.data().toString(), Qt::ElideMiddle, textRect.width() - 5);
  painter->drawText(textRect, Qt::AlignCenter, text);

  painter->restore();
}