#include "ThumbnailDelegate.hpp"
#include <QPixmap>
#include <QImage>
#include <QImageReader>
#include <QPixmapCache>
#include <QtConcurrent>
#include <QFileInfo>
#include <QDebug>
#include <QAbstractItemModel>
#include <QApplication>
#include <QPainter>

ThumbnailDelegate::ThumbnailDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
    // Cache auf ca. 300 MB erhöhen (Einheit ist KB)
    QPixmapCache::setCacheLimit(300 * 1024); 
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
    // Größe passend zum Grid im MainWindow (280x280 dort, hier 250x250 Bild + Rand)
    return QSize(250, 250);
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // 1. Pfad aus Spalte 1 holen (Pfad ist immer Spalte 1 im DB-Schema)
    QModelIndex pathIndex = index.model()->index(index.row(), 1);
    QString filePath = pathIndex.data().toString();
    QString cacheKey = "thumb_" + filePath;

    painter->save();
    painter->setClipRect(option.rect);

    // Hintergrund zeichnen
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    // 2. Bild aus Cache holen oder laden
    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        // --- CACHE HIT: Bild zeichnen ---
        QRect rect = option.rect;
        rect.setBottom(rect.bottom() - 30); // Platz für Text unten lassen

        QSize pSize = pixmap.size();
        pSize.scale(rect.size(), Qt::KeepAspectRatio);

        int x = rect.x() + (rect.width() - pSize.width()) / 2;
        int y = rect.y() + (rect.height() - pSize.height()) / 2;

        painter->drawPixmap(x, y, pSize.width(), pSize.height(), pixmap);
    } else {
        // --- CACHE MISS: Laden anstoßen ---
        
        // Platzhalter zeichnen
        QRect placeholderRect = option.rect.adjusted(10, 10, -10, -40);
        painter->setPen(Qt::lightGray);
        painter->drawRect(placeholderRect);
        
        QFont f = painter->font();
        f.setPointSize(8);
        painter->setFont(f);
        painter->drawText(option.rect.adjusted(0, -20, 0, 0), Qt::AlignCenter, "Lade...");

        // Viewport holen für das Update-Signal
        QWidget* viewport = const_cast<QWidget*>(option.widget);
        
        // Asynchron laden
        QtConcurrent::run([filePath, cacheKey, viewport]() {
            QFileInfo fi(filePath);
            if (!fi.exists()) return;

            QImageReader reader(filePath);
            if (!reader.canRead()) return;

            // Turbo-Modus: Nur benötigte Größe dekodieren
            QSize originalSize = reader.size();
            QSize targetSize(300, 300); // Etwas größer für Qualität
            
            if (originalSize.isValid() && (originalSize.width() > targetSize.width() || originalSize.height() > targetSize.height())) {
                reader.setScaledSize(originalSize.scaled(targetSize, Qt::KeepAspectRatio));
            }

            QImage img = reader.read();
            if (img.isNull()) return;

            // Update im Main-Thread
            if (viewport) {
                QMetaObject::invokeMethod(viewport, [viewport, cacheKey, img]() {
                    QPixmap pm = QPixmap::fromImage(img);
                    QPixmapCache::insert(cacheKey, pm);
                    viewport->update(); // Neuzeichnen erzwingen
                });
            }
        });
    }

    // 3. Overlay Infos zeichnen (Copyright / GPS)
    // Spalten-Indices basierend auf DatabaseManager Schema:
    // ... 8:copyright, 9:gps_decimal
    const int COL_COPYRIGHT = 8;
    const int COL_GPS = 9;

    int textY = option.rect.bottom() - 35; // Startposition über dem Dateinamen (von unten nach oben)

    // Schriftart für Overlays
    QFont overlayFont = painter->font();
    overlayFont.setPixelSize(10);
    overlayFont.setBold(true);
    painter->setFont(overlayFont);

    // Copyright (Rot)
    if (m_showCopyright) {
        QModelIndex idx = index.model()->index(index.row(), COL_COPYRIGHT);
        QString cp = idx.data().toString();
        if (!cp.isEmpty()) {
            painter->setPen(Qt::red);
            QRect rect = option.rect;
            rect.setBottom(textY);
            rect.setTop(textY - 15);
            painter->drawText(rect, Qt::AlignCenter, "© " + cp);
            textY -= 15; // Nächste Zeile weiter oben
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

    // 4. Dateiname zeichnen (ganz unten)
    QRect textRect = option.rect;
    textRect.setTop(textRect.bottom() - 30);
    
    // Schrift zurücksetzen für Dateiname
    painter->setFont(option.font);
    painter->setPen((option.state & QStyle::State_Selected) ? 
                    option.palette.highlightedText().color() : 
                    option.palette.text().color());
    
    QString text = option.fontMetrics.elidedText(index.data().toString(), Qt::ElideMiddle, textRect.width() - 5);
    painter->drawText(textRect, Qt::AlignCenter, text);

    painter->restore();
}