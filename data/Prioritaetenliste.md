<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Prioritätenliste: Optimierung von Sicherheit, Stabilität & Performance (`desktop-gallery`)](#priorit%C3%A4tenliste-optimierung-von-sicherheit-stabilit%C3%A4t--performance-desktop-gallery)
  - [🔴 Priorität 1: Kritische Absturzursachen & Thread-Safety (Sofortmaßnahmen)](#-priorit%C3%A4t-1-kritische-absturzursachen--thread-safety-sofortma%C3%9Fnahmen)
  - [🟠 Priorität 2: Hohe Sicherheitsrisiken & massive I/O-Engpässe](#-priorit%C3%A4t-2-hohe-sicherheitsrisiken--massive-io-engp%C3%A4sse)
  - [🟡 Priorität 3: Mittlere Performance-, Sicherheits- & Architektur-Optimierungen](#-priorit%C3%A4t-3-mittlere-performance--sicherheits---architektur-optimierungen)
  - [🟢 Priorität 4: Feinschliff & Ressourcen-Hygiene (Quick Wins)](#-priorit%C3%A4t-4-feinschliff--ressourcen-hygiene-quick-wins)
  - [Empfohlene Reihenfolge für Spec Kit Features (`/speckit.specify`)](#empfohlene-reihenfolge-f%C3%BCr-spec-kit-features-speckitspecify)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Prioritätenliste: Optimierung von Sicherheit, Stabilität & Performance (`desktop-gallery`)

**Referenz-Analyse:** `data/Code_Analysis_Security_Stability_Performance.md`  
**Ziel:** Strukturierte, schrittweise Abarbeitung der identifizierten Schwachstellen gemäß SpecDD (`desktop-gallery.sdd`) und GitHub Spec Kit.

---

## 🔴 Priorität 1: Kritische Absturzursachen & Thread-Safety (Sofortmaßnahmen)

Diese Punkte beheben potenzielle Programmabstürze (`SIGSEGV`), Race Conditions zwischen Worker- und GUI-Threads sowie Datenbank-Sperren.

- [x] **1.1 [STAB-01] Thread-sicheres Laden der Metadaten in `PictureWidget`**
  - **Auswirkung:** Verhindert zufällige Segfaults beim Öffnen der Bild-Detailansicht.
  - **Betroffene Dateien:** `include/picture_widget.h`, `src/picture_widget.cpp`
  - **Maßnahme:** `readSrcExif()`, `readSrcIptc()` und `readSrcXmp()` dürfen aus `QtConcurrent::run` heraus **nicht** direkt auf `ui->exifTableWidget`, `ui->iptcTableWidget`, `ui->xmpTableWidget` und `ui->tabWidget` zugreifen. Stattdessen Bilddatei im Hintergrund-Thread einmalig mit Exiv2 öffnen, Daten in eine reine C++-Struktur lesen und das UI-Update per `QMetaObject::invokeMethod(this, ...)` im GUI-Hauptthread ausführen.

- [x] **1.2 [STAB-02 & PERF-03] Absicherung & Deduplizierung im `ThumbnailDelegate`**
  - **Auswirkung:** Verhindert Use-After-Free-Abstürze (`viewport`) und beseitigt die ThreadPool-Überflutung beim Scrollen.
  - **Betroffene Dateien:** `include/ThumbnailDelegate.hpp`, `src/ThumbnailDelegate.cpp`
  - **Maßnahme:**
    1. `QPointer<QWidget> safeViewport(const_cast<QWidget *>(option.widget));` statt rohem `QWidget*`-Pointer im Lambda nutzen.
    2. In-Flight-Set (`mutable QSet<QString> m_loadingCacheKeys;`) ergänzen, damit für dasselbe Bild während des Ladevorgangs nur **ein** Task im `QThreadPool` gestartet wird.
    3. `reader.setAutoTransform(true)` für korrekte EXIF-Orientierung aktivieren.

- [x] **1.3 [STAB-04] SQLite-Concurrency & Indexer-Synchronisation bereinigen**
  - **Auswirkung:** Verhindert `SQLITE_BUSY`-Fehler und Datenbank-Korruption beim parallelen Lesen/Schreiben.
  - **Betroffene Dateien:** `src/ImageIndexer.cpp`, `include/MainWindow.hpp`, `src/MainWindow.cpp`
  - **Maßnahme:**
    1. `PRAGMA journal_mode = MEMORY;` und `PRAGMA synchronous = OFF;` in `ImageIndexer::run()` entfernen (der in `DatabaseManager::init()` gesetzte `WAL`-Modus bleibt erhalten).
    2. `PRAGMA busy_timeout = 5000;` für die Indexer-Verbindung setzen.
    3. Laufende `ImageIndexer`-Tasks beim Beenden von `MainWindow` sauber via `requestInterruption()` stoppen.

---

## 🟠 Priorität 2: Hohe Sicherheitsrisiken & massive I/O-Engpässe

Diese Punkte schließen aktive Sicherheitslücken (DOM-XSS, Klartext-Passwörter) und beseitigen die größten Performance-Bremsen bei der Bildverarbeitung und beim Geocoding.

- [x] **2.1 [SEC-01] Schutz vor HTML-/JavaScript-Injection (DOM-XSS) in Kartenansichten**
  - **Auswirkung:** Verhindert Code-Ausführung in `QWebEngineView` durch präparierte Bild-Dateinamen.
  - **Betroffene Dateien:** `src/MapWindow.cpp`, `src/picture_widget.cpp`
  - **Maßnahme:** Dateinamen vor dem Einbetten in HTML-Popups mit `.toHtmlEscaped()` maskieren und in `PictureWidget::loadMap()` den Titel als JSON-kodierten String (`QJsonDocument`) statt per roher String-Konkatenation an JavaScript übergeben.

- [x] **2.2 [SEC-02 & SEC-04] Härtung von `UploadManager` (Passwort-Handling, HTTPS & Multipart)**
  - **Auswirkung:** Kein dauerhaftes Vorhalten des Klartext-Passworts im RAM; Schutz vor unverschlüsseltem Login und defekten Multipart-Headern.
  - **Betroffene Dateien:** `include/UploadManager.hpp`, `src/UploadManager.cpp`, `src/MainWindow.cpp`
  - **Maßnahme:**
    1. Unbenutzte Member-Variablen `m_lastPass` und `m_lastUser` aus `UploadManager` entfernen.
    2. Warnhinweis vor dem Login anzeigen, falls eine Remote-Upload-URL (nicht `localhost`/`127.0.0.1`) unverschlüsseltes `http://` statt `https://` nutzt.
    3. Anführungszeichen und Steuerzeichen im Dateinamen beim `Content-Disposition`-Header in `UploadManager::startNextUpload()` bereinigen.

- [x] **2.3 [PERF-01] Redundante Exiv2- und Bild-Ladevorgänge in `Photo` eliminieren**
  - **Auswirkung:** Halbierung der I/O-Zeit beim Ordnerscan (`ImageIndexer`) und bis zu 6x schnellerer WebP-Multi-Size-Export.
  - **Betroffene Dateien:** `include/rz_photo.hpp`, `src/rz_photo.cpp`, `src/ImageIndexer.cpp`
  - **Maßnahme:**
    1. Kombinierte Methode `Photo::readSummary()` implementieren, die Copyright, GPS und Aufnahmedatum in **einem einzigen** `Exiv2::ImageFactory::open()`-Aufruf liest (statt 2–3 separaten Dateiöffnungen im `ImageIndexer`).
    2. `Photo::writeToAllCopyrightOwner()` auf einen einzigen Schreibvorgang (`writeAllMetadata()`) umstellen (statt 7 einzelner Datei-Schreibvorgänge).
    3. In `Photo::convertImages()` das Quellbild (`QImage`) und den Zeitstempel einmalig im RAM laden und daraus alle Zielgrößen skalieren.

- [x] **2.4 [STAB-03 & STAB-05] Asynchronisierung von Regeocode-Batch & WebP-Export**
  - **Auswirkung:** Kein Einfrieren der GUI während Batch-Geocoding oder WebP-Export; kein Stack-Overflow-Risiko durch Rekursion.
  - **Betroffene Dateien:** `src/MainWindow.cpp`, `src/picture_widget.cpp`
  - **Maßnahme:**
    1. In `MainWindow::processNextRegeocodeLookup()` `ConfigLoader`, Adapter und `ReverseGeocoder` nur einmal pro Batch initialisieren und die Abarbeitung iterativ in einem Hintergrund-Task statt rekursiv im GUI-Thread durchführen.
    2. In `PictureWidget::exportSrcImgToWebpThread()` das blockierende `future.result()` durch einen asynchronen `QFutureWatcher<bool>` ersetzen.

---

## 🟡 Priorität 3: Mittlere Performance-, Sicherheits- & Architektur-Optimierungen

Diese Punkte verbessern die Start- und Bedien-Performance bei großen Bildarchiven sowie die Build- und Thread-Sauberkeit.

- [x] **3.1 [PERF-02] Persistenter SQLite-Index statt Löschen bei jedem Programmstart**
  - **Auswirkung:** Sofortige Verfügbarkeit bereits gescannter Ordner nach App-Neustart.
  - **Betroffene Dateien:** `src/DatabaseManager.cpp`, `src/ImageIndexer.cpp`, `src/MainWindow.cpp`
  - **Maßnahme:** `QFile::remove(m_dbPath)` aus `DatabaseManager::init()` entfernen; stattdessen `file_mtime` in der Tabelle `images` berücksichtigen und im Menü eine explizite Aktion „Cache leeren" (`DatabaseManager::clearDatabase()`) anbieten.

- [x] **3.2 [PERF-04] Transaktionsbündelung & Hintergrund-Ausführung für Bulk-Metadaten-Aktionen**
  - **Auswirkung:** Bis zu 50x schnellere Datenbank-Updates und flüssige GUI beim Schreiben von Default-Metadaten oder Umbenennen vieler Bilder.
  - **Betroffene Dateien:** `src/MainWindow.cpp`
  - **Maßnahme:** `renameSelectedImages()`, `writeDefaultCopyrightToSelection()`, `writeDefaultGpsToSelection()` und `writeAllDefaultsToSelection()` mit einer SQLite-Transaktion (`db.transaction()` / `db.commit()`) umschließen und bei großen Auswahlen asynchron mit Fortschrittsanzeige ausführen.

- [x] **3.3 [SEC-03] Supply-Chain-Pinning & Subresource Integrity (SRI)**
  - **Auswirkung:** Reproduzierbare, manipulationssichere Builds und abgesicherte externe Skripte.
  - **Betroffene Dateien:** `CMakeLists.txt`, `src/MapWindow.cpp`, `src/picture_widget.cpp`
  - **Maßnahme:**
    1. In `CMakeLists.txt` bei `inifile-cpp` `GIT_TAG master` durch einen festen Commit-Hash ersetzen.
    2. Bei den Leaflet-CDN-Einbindungen (`unpkg.com`) `integrity`- und `crossorigin`-Attribute ergänzen (oder Leaflet lokal in `resources.qrc` bündeln).

- [x] **3.4 [STAB-06] Thread-sichere Zeitfunktion in `QuotaManager` & `curl_global_init`**
  - **Auswirkung:** Vermeidung von Data Races bei parallelen `regeocode`-Aufrufen.
  - **Betroffene Dateien:** `src/regeocode/quota_manager.cpp`, `src/regeocode/http_client.cpp`, `src/main.cpp`
  - **Maßnahme:** `std::localtime` durch threadsicheres `localtime_r` / `localtime_s` (oder C++20/26 `std::chrono`-Formatierung) ersetzen; `curl_global_init(CURL_GLOBAL_DEFAULT)` einmalig beim Programmstart ausführen.

---

## 🟢 Priorität 4: Feinschliff & Ressourcen-Hygiene (Quick Wins)

- [x] **4.1 [PERF-05] Flüssiges Skalieren in `PictureWidget::resizeEvent()`**
  - **Auswirkung:** Ruckelfreies Vergrößern/Verkleinern des Bildbetrachters auch bei 50-MP-Fotos.
  - **Betroffene Dateien:** `src/picture_widget.cpp`, `include/picture_widget.h`
  - **Maßnahme:** Während des laufenden Resize-Vorgangs `Qt::FastTransformation` nutzen und `Qt::SmoothTransformation` verzögert über einen Single-Shot-`QTimer` (100 ms) berechnen.

- [x] **4.2 [STAB-07] Speicher-Ownership (`QMenu` & `QPixmap`) in `PictureWidget` modernisieren**
  - **Auswirkung:** Beseitigt kleinere Memory-Leaks bei jedem Öffnen/Schließen von `PictureWidget`.
  - **Betroffene Dateien:** `src/picture_widget.cpp`, `include/picture_widget.h`
  - **Maßnahme:** `rotateMnu = new QMenu(this);` und `exportMnu = new QMenu(this);` mit Parent initialisieren; `QPixmap *picture` durch einen direkten Value-Member `QPixmap picture;` ersetzen.

---

## Empfohlene Reihenfolge für Spec Kit Features (`/speckit.specify`)

| Arbeitspaket | Enthaltene Punkte | Vorgeschlagener Feature-Name (`specs/features/...`) |
| :--- | :--- | :--- |
| **Paket 1** | 1.1, 1.2, 1.3, 4.2 | `stability-thread-safety-core` |
| **Paket 2** | 2.1, 2.2, 3.3, 3.4 | `security-hardening-web-upload` |
| **Paket 3** | 2.3, 2.4, 3.1, 3.2, 4.1 | `performance-io-indexing-batch` |
