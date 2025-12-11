#include "MapWindow.hpp"
#include "rz_photo.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QWebEngineView>

MapWindow::MapWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle(tr("Map View"));
  resize(1024, 768);

  QWidget *central = new QWidget(this);
  setCentralWidget(central);
  QVBoxLayout *layout = new QVBoxLayout(central);
  layout->setContentsMargins(0, 0, 0, 0);

  m_webView = new QWebEngineView(this);
  layout->addWidget(m_webView);
}

void MapWindow::showImages(const QList<QString> &filePaths) {
  QString html = generateHtml(filePaths);
  m_webView->setHtml(html);
}

QString MapWindow::generateHtml(const QList<QString> &filePaths) {
  // 1. Daten sammeln (Lat, Lon, Name)
  QJsonArray markers;

  for (const QString &path : filePaths) {
    Photo p(path);
    QPointF gps =
        p.getGpsLatLon(); // X=Lat, Y=Lon (laut deiner Implementierung)

    // Filter: Nur gültige Koordinaten
    if (qAbs(gps.x()) > 0.0001 || qAbs(gps.y()) > 0.0001) {
      QJsonObject m;
      m["lat"] = gps.x();
      m["lng"] = gps.y();
      m["title"] = QFileInfo(path).fileName();
      // Optional: Mehr Infos ins Popup (z.B. Thumbnail Pfad, aber local file
      // access im Browser ist tricky)
      markers.append(m);
    }
  }

  QString jsonString = QJsonDocument(markers).toJson(QJsonDocument::Compact);

  // 2. HTML Template mit Leaflet.js
  // Wir nutzen CDN Links für Leaflet. Internetverbindung erforderlich.
  QString html = R"(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8">
        <title>Map</title>
        <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
        <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
        <style>
            body, html, #map { height: 100%; margin: 0; padding: 0; }
        </style>
    </head>
    <body>
        <div id="map"></div>
        <script>
            // Karte initialisieren
            var map = L.map('map').setView([0, 0], 2);

            // OpenStreetMap Layer
            L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
            }).addTo(map);

            // Daten aus C++
            var data = )" +
                 jsonString + R"(;
            
            var bounds = [];
            var group = L.featureGroup();

            data.forEach(function(item) {
                var marker = L.marker([item.lat, item.lng]).addTo(group);
                marker.bindPopup("<b>" + item.title + "</b><br>" + item.lat.toFixed(5) + ", " + item.lng.toFixed(5));
            });

            if (data.length > 0) {
                group.addTo(map);
                map.fitBounds(group.getBounds().pad(0.1));
            } else {
                // Default View (Europa), falls leer
                map.setView([51.16, 10.45], 5);
            }
        </script>
    </body>
    </html>
    )";

  return html;
}