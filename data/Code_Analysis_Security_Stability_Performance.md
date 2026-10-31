<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Sourcecode-Analyse: Sicherheit, Stabilität & Performance (`desktop-gallery`)](#sourcecode-analyse-sicherheit-stabilit%C3%A4t--performance-desktop-gallery)
  - [Management Summary](#management-summary)
    - [Übersicht nach Kritikalität](#%C3%9Cbersicht-nach-kritikalit%C3%A4t)
  - [1. Stabilität & Thread-Safety (Stability)](#1-stabilit%C3%A4t--thread-safety-stability)
    - [STAB-01 (🔴 Kritisch): Direkte UI-Manipulation aus Hintergrund-Threads in `PictureWidget`](#stab-01--kritisch-direkte-ui-manipulation-aus-hintergrund-threads-in-picturewidget)
    - [STAB-02 (🔴 Kritisch): Dangling-Pointer-Risiko & Task-Flutung in `ThumbnailDelegate::paint()`](#stab-02--kritisch-dangling-pointer-risiko--task-flutung-in-thumbnaildelegatepaint)
    - [STAB-03 (🟠 Hoch): Blockierender GUI-Thread, Ressourcen-Overhead & unendliche Rekursion in `processNextRegeocodeLookup()`](#stab-03--hoch-blockierender-gui-thread-ressourcen-overhead--unendliche-rekursion-in-processnextregeocodelookup)
    - [STAB-04 (🟠 Hoch): SQLite Journal-Mode-Konflikt zwischen `DatabaseManager` und `ImageIndexer`](#stab-04--hoch-sqlite-journal-mode-konflikt-zwischen-databasemanager-und-imageindexer)
    - [STAB-05 (🟡 Mittel): Synchrones Blockieren durch `future.result()` in `PictureWidget::exportSrcImgToWebpThread()`](#stab-05--mittel-synchrones-blockieren-durch-futureresult-in-picturewidgetexportsrcimgtowebpthread)
    - [STAB-06 (🟡 Mittel): Fehlende `curl_global_init()`-Initialisierung & nicht-threadsicheres `std::localtime`](#stab-06--mittel-fehlende-curl_global_init-initialisierung--nicht-threadsicheres-stdlocaltime)
    - [STAB-07 (🟢 Niedrig): Memory-Leaks bei `QMenu`-Objekten in `PictureWidget`](#stab-07--niedrig-memory-leaks-bei-qmenu-objekten-in-picturewidget)
  - [2. Sicherheit (Security)](#2-sicherheit-security)
    - [SEC-01 (🟠 Hoch): DOM-XSS / HTML- & JavaScript-Injection in `MapWindow` und `PictureWidget`](#sec-01--hoch-dom-xss--html---javascript-injection-in-mapwindow-und-picturewidget)
    - [SEC-02 (🟠 Hoch): Unverschlüsselte Übertragung bei HTTP-URLs & unnötige Passwort-Speicherung im RAM](#sec-02--hoch-unverschl%C3%BCsselte-%C3%9Cbertragung-bei-http-urls--unn%C3%B6tige-passwort-speicherung-im-ram)
    - [SEC-03 (🟡 Mittel): Supply-Chain-Sicherheit in `CMakeLists.txt` & fehlendes SRI beim Leaflet-CDN](#sec-03--mittel-supply-chain-sicherheit-in-cmakeliststxt--fehlendes-sri-beim-leaflet-cdn)
    - [SEC-04 (🟡 Mittel): Multipart-Header-Formatierung & SQL-Filter-Konstruktion](#sec-04--mittel-multipart-header-formatierung--sql-filter-konstruktion)
  - [3. Performance & Ressourceneffizienz](#3-performance--ressourceneffizienz)
    - [PERF-01 (🔴 Kritisch): Massiv redundante Datei- und Exiv2-Zugriffe in `Photo` und `ImageIndexer`](#perf-01--kritisch-massiv-redundante-datei--und-exiv2-zugriffe-in-photo-und-imageindexer)
    - [PERF-02 (🟠 Hoch): Löschen der gesamten SQLite-Datenbank bei jedem Programmstart](#perf-02--hoch-l%C3%B6schen-der-gesamten-sqlite-datenbank-bei-jedem-programmstart)
    - [PERF-03 (🟠 Hoch): Ungedrosselte `QImageReader`-Tasks in `ThumbnailDelegate::paint()`](#perf-03--hoch-ungedrosselte-qimagereader-tasks-in-thumbnaildelegatepaint)
    - [PERF-04 (🟡 Mittel): Synchrone Bulk-Schreiboperationen im GUI-Thread & Einzel-SQL-Queries](#perf-04--mittel-synchrone-bulk-schreiboperationen-im-gui-thread--einzel-sql-queries)
    - [PERF-05 (🟡 Mittel): Speichern und wiederholtes Skalieren des vollauflösenden Bildes in `PictureWidget`](#perf-05--mittel-speichern-und-wiederholtes-skalieren-des-vollaufl%C3%B6senden-bildes-in-picturewidget)
  - [4. Empfohlene Priorisierung (Roadmap für Spec Kit Tasks)](#4-empfohlene-priorisierung-roadmap-f%C3%BCr-spec-kit-tasks)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Sourcecode-Analyse: Sicherheit, Stabilität & Performance (`desktop-gallery`)

**Projekt:** `desktop-gallery` (`web-gallery_desktop-gallery`)  
**Technologie-Stack:** C++26 (Linux) / C++23 (Windows, macOS), Qt6, SQLite3, Exiv2, libcurl  
**Datum:** 2026-10-31  

---

## Management Summary

Die Anwendung verfügt über eine klare funktionale Trennung (UI, `DatabaseManager`, `ImageIndexer`, `Photo`/Exiv2-Wrapper, `regeocode`, `UploadManager`). Bei der Tiefenanalyse des Quellcodes wurden jedoch mehrere **kritische Threading- und Stabilitätsprobleme** (insbesondere direkte GUI-Manipulationen aus Hintergrund-Threads), **Sicherheitsrisiken** (HTML/JS-Injection in `QWebEngineView`, Klartext-Passworthaltung im RAM, ungepinnte Abhängigkeiten) sowie **erhebliche Performance-Engpässe** (mehrfaches Öffnen/Schreiben derselben Bilddatei über Exiv2, Löschen der Cache-Datenbank beim Programmstart, ungedrosselte Thumbnail-Tasks beim Scrollen) identifiziert.

### Übersicht nach Kritikalität

| ID | Kategorie | Schweregrad | Komponente / Datei | Kurzbeschreibung |
| :--- | :--- | :--- | :--- | :--- |
| **STAB-01** | Stabilität | 🔴 Kritisch | `src/picture_widget.cpp` | Direkter Zugriff auf `QTableWidget` / UI-Elemente aus 3 `QtConcurrent`-Worker-Threads (Undefined Behavior / Segfault-Gefahr) |
| **STAB-02** | Stabilität | 🔴 Kritisch | `src/ThumbnailDelegate.cpp` | Dangling-Pointer-Risiko (`viewport`) & ungedrosseltes Task-Spawning in `paint()` |
| **STAB-03** | Stabilität | 🟠 Hoch | `src/MainWindow.cpp` | Blockierender Netzwerk-/I/O-Aufruf im GUI-Thread & unendliche Rekursion in `processNextRegeocodeLookup()` |
| **STAB-04** | Stabilität | 🟠 Hoch | `src/ImageIndexer.cpp` | SQLite `PRAGMA journal_mode = MEMORY` kollidiert mit `WAL`-Modus der Hauptverbindung |
| **STAB-05** | Stabilität | 🟡 Mittel | `src/picture_widget.cpp` | `future.result()` direkt nach `QtConcurrent::run()` blockiert den GUI-Thread synchron |
| **STAB-06** | Stabilität | 🟡 Mittel | `src/regeocode/*` | Fehlende `curl_global_init()`-Initialisierung und nicht-threadsicheres `std::localtime` |
| **STAB-07** | Stabilität | 🟢 Niedrig | `src/picture_widget.cpp` | Memory-Leaks bei `QMenu`-Instanzen (`rotateMnu`, `exportMnu`) ohne Parent |
| **SEC-01** | Sicherheit | 🟠 Hoch | `src/MapWindow.cpp`, `src/picture_widget.cpp` | DOM-XSS / HTML- & JS-Injection über manipulierte Dateinamen in `QWebEngineView` |
| **SEC-02** | Sicherheit | 🟠 Hoch | `src/UploadManager.cpp`, `src/MainWindow.cpp` | Keine HTTPS-Validierung für Remote-Logins & unnötige Speicherung des Klartext-Passworts (`m_lastPass`) |
| **SEC-03** | Sicherheit | 🟡 Mittel | `CMakeLists.txt`, `MapWindow.cpp` | Supply-Chain: `GIT_TAG master` bei `inifile-cpp` & fehlende Subresource Integrity (SRI) für Leaflet-CDN |
| **SEC-04** | Sicherheit | 🟡 Mittel | `src/UploadManager.cpp`, `src/MainWindow.cpp` | Multipart-Header-Injection über Dateinamen & manuelles SQL-String-Escaping in `setFilter()` |
| **PERF-01** | Performance | 🔴 Kritisch | `src/rz_photo.cpp`, `src/ImageIndexer.cpp` | Redundantes Öffnen/Parsen/Schreiben derselben Datei via Exiv2 (2x pro Datei beim Indexing, 7x beim Copyright-Schreiben) |
| **PERF-02** | Performance | 🟠 Hoch | `src/DatabaseManager.cpp` | Komplettes Löschen der SQLite-Cache-Datenbank (`gallery_cache.db`) bei jedem Anwendungsstart |
| **PERF-03** | Performance | 🟠 Hoch | `src/ThumbnailDelegate.cpp` | Fehlendes In-Flight-Tracking erzeugt hunderte doppelte `QImageReader`-Jobs beim Scrollen |
| **PERF-04** | Performance | 🟡 Mittel | `src/MainWindow.cpp` | Synchrone Bulk-Metadaten-Operationen im GUI-Thread ohne SQLite-Transaktionsklammer |
| **PERF-05** | Performance | 🟡 Mittel | `src/picture_widget.cpp` | Vollauflösendes `QPixmap` im RAM & teure `SmoothTransformation` bei jedem `resizeEvent` |

---

## 1. Stabilität & Thread-Safety (Stability)

### STAB-01 (🔴 Kritisch): Direkte UI-Manipulation aus Hintergrund-Threads in `PictureWidget`
- **Fundstelle:** `src/picture_widget.cpp` (Zeilen 93–97 sowie 221–431: `readSrcExif()`, `readSrcIptc()`, `readSrcXmp()`)
- **Problem:** In `PictureWidget::setImage()` werden drei Methoden parallel über `QtConcurrent::run` in Worker-Threads gestartet:
  ```cpp
  QFuture<void> futureExif = QtConcurrent::run(&PictureWidget::readSrcExif, this);
  QFuture<void> futureIptc = QtConcurrent::run(&PictureWidget::readSrcIptc, this);
  QFuture<void> futureXmp  = QtConcurrent::run(&PictureWidget::readSrcXmp, this);
  ```
  Innerhalb dieser drei Funktionen werden direkt `ui->exifTableWidget`, `ui->iptcTableWidget`, `ui->xmpTableWidget` sowie `ui->tabWidget->setTabVisible(...)` verändert (`insertRow`, `setItem`, `resizeColumnsToContents`).  
  In Qt6 dürfen `QWidget`-Subklassen **ausschließlich im GUI-Hauptthread** gelesen und verändert werden. Gleichzeitige Zugriffe aus 3 Worker-Threads führen zu Race Conditions in der internen Qt-Event-/Paint-Engine und zu sporadischen Abstürzen (`SIGSEGV`).
- **Verbesserungsvorschlag:**
  Im Hintergrund-Thread ausschließlich die Metadaten via Exiv2 in eine reine C++-Datenstruktur (z. B. `QVector<std::tuple<QString, QString, QString>>`) einlesen und das Befüllen der `QTableWidget`-Instanzen per `QMetaObject::invokeMethod(this, ...)` im GUI-Thread ausführen. Zudem sollte Exiv2 die Datei nur **einmal** öffnen und alle drei Container (EXIF, IPTC, XMP) in einem Durchgang lesen.

---

### STAB-02 (🔴 Kritisch): Dangling-Pointer-Risiko & Task-Flutung in `ThumbnailDelegate::paint()`
- **Fundstelle:** `src/ThumbnailDelegate.cpp` (Zeilen 80–112)
- **Problem:**
  ```cpp
  QWidget *viewport = const_cast<QWidget *>(option.widget);
  QThreadPool::globalInstance()->start([filePath, cacheKey, viewport]() {
      ...
      if (viewport) {
          QMetaObject::invokeMethod(viewport, [viewport, cacheKey, img]() {
              QPixmap pm = QPixmap::fromImage(img);
              QPixmapCache::insert(cacheKey, pm);
              viewport->update();
          });
      }
  });
  ```
  1. `viewport` wird als roher Zeiger gecaptured. Wird die View geschlossen oder zerstört, während noch Tasks im `QThreadPool` laufen, zeigt `viewport` auf freigegebenen Speicher (Use-After-Free).
  2. Da `paint()` bei jedem Neuzeichnen (z. B. Mausbewegung, Scrollen, `viewport->update()` eines fertigen Thumbnails) aufgerufen wird, wird für jedes noch nicht im `QPixmapCache` befindliche Bild immer wieder ein neuer ThreadPool-Task gestartet.
- **Verbesserungsvorschlag:**
  - `QPointer<QWidget> safeViewport(const_cast<QWidget *>(option.widget));` verwenden und im Callback prüfen.
  - Ein `mutable QSet<QString> m_loadingPaths;` im `ThumbnailDelegate` führen: Vor dem Starten des Tasks prüfen, ob `m_loadingPaths.contains(cacheKey)` bereits wahr ist. Falls ja, keinen neuen Task starten. Im Main-Thread-Callback den Key nach `QPixmapCache::insert` wieder aus `m_loadingPaths` entfernen.

---

### STAB-03 (🟠 Hoch): Blockierender GUI-Thread, Ressourcen-Overhead & unendliche Rekursion in `processNextRegeocodeLookup()`
- **Fundstelle:** `src/MainWindow.cpp` (Zeilen 847–951)
- **Problem:**
  1. **Synchrone Blockade:** `geocoder.reverse_geocode_fallback()` und `geocoder.reverse_geocode_json()` führen synchrone `libcurl`-Aufrufe direkt im GUI-Thread aus. Während des Batch-Geolookups friert das gesamte Fenster ein.
  2. **Rekursiver Selbstaufruf:** Am Ende der Funktion (Zeilen 861 & 950) ruft `processNextRegeocodeLookup()` sich selbst synchron rekursiv auf. Bei großen Bildmengen wächst der Call-Stack linear mit der Anzahl der Bilder (Stack-Overflow-Gefahr).
  3. **Wiederholte Re-Initialisierung:** Für jedes einzelne Bild in der Queue wird die INI-Datei zweimal von der Festplatte geparst (`ConfigLoader` + `ini::IniFile`), alle 5 Adapter neu allokiert und `QuotaManager` neu geladen.
- **Verbesserungsvorschlag:**
  - Konfiguration und `ReverseGeocoder` einmalig in `startGeoNamesLookup()` initialisieren.
  - Die Batch-Abarbeitung in einen Hintergrund-Worker (`QtConcurrent::run` mit `QFutureWatcher`) mit einer normalen `while`-Schleife (statt Rekursion) auslagern und Fortschritt per Signal an die GUI melden.

---

### STAB-04 (🟠 Hoch): SQLite Journal-Mode-Konflikt zwischen `DatabaseManager` und `ImageIndexer`
- **Fundstelle:** `src/DatabaseManager.cpp` (Zeilen 78–79) vs. `src/ImageIndexer.cpp` (Zeilen 50–52)
- **Problem:**
  - `DatabaseManager::init()` konfiguriert die Datenbank mit `PRAGMA journal_mode = WAL;` und `PRAGMA synchronous = NORMAL;`.
  - `ImageIndexer::run()` öffnet dieselbe Datenbankdatei über eine zweite Verbindung und führt `PRAGMA synchronous = OFF;` sowie `PRAGMA journal_mode = MEMORY;` aus.
  - `journal_mode` ist eine datenbankweite Eigenschaft in SQLite. Der Versuch, aus einem Hintergrund-Thread von `WAL` auf `MEMORY` umzuschalten, während `m_galleryModel` im Hauptthread die Datenbank offen hält, führt zu `SQLITE_BUSY`-Fehlern oder bei Programmabsturz während des Scans zu einer korrupten Datenbankdatei.
  - Zudem kann der Benutzer mehrfach hintereinander auf „Open Folder" klicken, wodurch mehrere `ImageIndexer`-Instanzen gleichzeitig schreibend auf SQLite zugreifen.
- **Verbesserungsvorschlag:**
  - In `ImageIndexer::run()` `PRAGMA journal_mode = MEMORY;` und `PRAGMA synchronous = OFF;` entfernen (der `WAL`-Modus zusammen mit `db.transaction()` / `db.commit()` ist bereits extrem schnell und threadsicher für 1 Writer + N Reader).
  - `PRAGMA busy_timeout = 5000;` setzen.
  - Vor dem Starten eines neuen `ImageIndexer` prüfen, ob bereits ein Scan läuft (oder diese über eine Queue serialisieren) und beim Schließen von `MainWindow` laufende Indexer per `requestInterruption()` sauber abbrechen.

---

### STAB-05 (🟡 Mittel): Synchrones Blockieren durch `future.result()` in `PictureWidget::exportSrcImgToWebpThread()`
- **Fundstelle:** `src/picture_widget.cpp` (Zeilen 707–708)
- **Problem:**
  ```cpp
  QFuture<bool> future = QtConcurrent::run(&Photo::convertImages, phot, 75);
  oknok = future.result(); // Blockiert den GUI-Thread sofort bis zum Ende!
  ```
  Der Aufruf von `future.result()` wartet synchron im Hauptthread auf das Ende der Konvertierung aller Auflösungen.
- **Verbesserungsvorschlag:**
  Einen `QFutureWatcher<bool>` verwenden (oder `future.then(this, [this](bool ok) { ... })` ab Qt 6), damit der GUI-Thread während der Bildkonvertierung reaktionsfähig bleibt.

---

### STAB-06 (🟡 Mittel): Fehlende `curl_global_init()`-Initialisierung & nicht-threadsicheres `std::localtime`
- **Fundstellen:**
  - `src/regeocode/http_client.cpp` (Zeile 25)
  - `src/regeocode/quota_manager.cpp` (Zeile 35)
- **Problem:**
  - `curl_easy_init()` ruft intern `curl_global_init()` auf, falls dies noch nicht geschehen ist. Laut `libcurl`-Dokumentation ist diese implizite Initialisierung **nicht threadsicher**, wenn (wie in `ReverseGeocoder::batch_reverse_geocode` über `std::async`) mehrere Threads gleichzeitig HTTP-Requests starten.
  - `std::localtime(&in_time_t)` in `QuotaManager::get_current_date()` nutzt einen globalen statischen Puffer und ist bei parallelen Aufrufen nicht threadsicher (Data Race).
- **Verbesserungsvorschlag:**
  - `curl_global_init(CURL_GLOBAL_DEFAULT)` einmalig beim Programmstart (z. B. in `main()`) aufrufen oder statt `libcurl` durchgängig Qt6 (`QNetworkAccessManager`) nutzen.
  - In C++26 / C++20 `std::format("{:%Y-%m-%d}", std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()))` oder `localtime_r` (POSIX) / `localtime_s` (Windows) verwenden.

---

### STAB-07 (🟢 Niedrig): Memory-Leaks bei `QMenu`-Objekten in `PictureWidget`
- **Fundstelle:** `src/picture_widget.cpp` (Zeilen 450, 482)
- **Problem:** `rotateMnu = new QMenu();` und `exportMnu = new QMenu();` werden ohne `this` als Parent erstellt und im Destruktor `~PictureWidget()` nicht gelöscht. Da `PictureWidget` mit `Qt::WA_DeleteOnClose` bei jedem Doppelklick auf ein Bild neu erstellt und beim Schließen gelöscht wird, leaken diese Menüs bei jedem geöffneten Bild.
- **Verbesserungsvorschlag:** `rotateMnu = new QMenu(this);` und `exportMnu = new QMenu(this);` über die Qt-Parent-Child-Ownership verwalten; außerdem `std::unique_ptr<QPixmap>` (oder `QPixmap` als Value-Member statt Raw-Pointer `QPixmap *picture`) nutzen.

---

## 2. Sicherheit (Security)

### SEC-01 (🟠 Hoch): DOM-XSS / HTML- & JavaScript-Injection in `MapWindow` und `PictureWidget`
- **Fundstellen:**
  - `src/picture_widget.cpp` (Zeilen 127, 158–159):
    ```cpp
    QString title = QFileInfo(pathToImage).fileName();
    ...
    .bindPopup("<b>)" + title + R"(</b>")
    ```
  - `src/MapWindow.cpp` (Zeilen 61, 104):
    ```javascript
    marker.bindPopup("<b>" + item.title + "</b><br>" + ...);
    ```
- **Problem:** Dateinamen aus dem Dateisystem werden ungefiltert in HTML/JavaScript-Strings eingebettet, die in einer Chromium-basierten `QWebEngineView` ausgeführt werden. Ein Bild mit einem Dateinamen wie `test"); alert(1); //.jpg` oder `<img src=x onerror=...>.jpg` bricht aus dem JS/HTML-Kontext aus und führt beliebigen JavaScript-Code innerhalb der `QWebEngineView` aus.
- **Verbesserungsvorschlag:**
  - In `PictureWidget::loadMap()` und `MapWindow::generateHtml()` den Dateinamen vor dem Einbetten mit `title.toHtmlEscaped()` maskieren und für die Übergabe an JavaScript `QJsonDocument` nutzen (sodass Anführungszeichen und Backslashes sicher als JSON-String serialisiert werden).

---

### SEC-02 (🟠 Hoch): Unverschlüsselte Übertragung bei HTTP-URLs & unnötige Passwort-Speicherung im RAM
- **Fundstellen:**
  - `src/UploadManager.cpp` (Zeilen 41–55) & `include/UploadManager.hpp` (`m_lastPass`)
  - `src/MainWindow.cpp` (Zeilen 265, 1207)
- **Problem:**
  1. In `UploadManager::login(user, pass)` wird das Passwort in der Member-Variable `m_lastPass` dauerhaft für die gesamte Laufzeit der Anwendung im Speicher gehalten, obwohl `m_lastPass` im gesamten restlichen Code **nirgendwo mehr verwendet wird** (für das Token-Refresh wird ausschließlich `m_refreshToken` genutzt).
  2. Wenn der Benutzer eine Remote-URL mit `http://` (statt `https://`) konfiguriert (der Default-Platzhalter ist `http://localhost:8080`), werden Benutzername, Passwort und Bearer-JWT-Tokens im Klartext über das Netzwerk gesendet.
- **Verbesserungsvorschlag:**
  - Member-Variable `m_lastPass` (und `m_lastUser`) aus `UploadManager` entfernen.
  - Vor dem Login prüfen, ob die Ziel-URL bei Nicht-Localhost-Adressen `https://` verwendet, und andernfalls eine deutliche Sicherheitswarnung anzeigen.

---

### SEC-03 (🟡 Mittel): Supply-Chain-Sicherheit in `CMakeLists.txt` & fehlendes SRI beim Leaflet-CDN
- **Fundstellen:**
  - `CMakeLists.txt` (Zeilen 50–55):
    ```cmake
    FetchContent_Declare(
      inifile-cpp
      GIT_REPOSITORY https://github.com/Rookfighter/inifile-cpp.git
      GIT_TAG master)
    ```
  - `src/MapWindow.cpp` (Zeilen 78–79) & `src/picture_widget.cpp` (Zeilen 138–139)
- **Problem:**
  1. `GIT_TAG master` lädt beim CMake-Configure-Schritt immer den neuesten Commit vom `master`-Branch herunter. Dies gefährdet reproduzierbare Builds und öffnet ein Supply-Chain-Risiko bei Kompromittierung des Drittanbieter-Repos.
  2. Leaflet (`https://unpkg.com/leaflet@1.9.4/dist/leaflet.js`) wird zur Laufzeit ohne `integrity` (Subresource Integrity) geladen.
- **Verbesserungsvorschlag:**
  - In `CMakeLists.txt` `inifile-cpp` auf einen festen Commit-Hash oder Release-Tag pinnen.
  - Bei den `<link>`- und `<script>`-Tags für Leaflet die offiziellen `integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo="` und `crossorigin=""`-Attribute ergänzen (oder `leaflet.js`/`leaflet.css` direkt lokal in `resources.qrc` einbetten – das ermöglicht zudem Kartendarstellung, wenn `unpkg.com` blockiert oder langsam ist).

---

### SEC-04 (🟡 Mittel): Multipart-Header-Formatierung & SQL-Filter-Konstruktion
- **Fundstellen:**
  - `src/UploadManager.cpp` (Zeile 113):
    ```cpp
    QVariant("form-data; name=\"photo\"; filename=\"" + fileName + "\"")
    ```
  - `src/MainWindow.cpp` (Zeile 515):
    ```cpp
    QString dbFilter = QString("folder_path = '%1'").arg(path.replace("'", "''"));
    m_galleryModel->setFilter(dbFilter);
    ```
- **Problem:**
  - Enthält ein Dateiname unter Linux ein doppeltes Anführungszeichen `"` oder Zeilenumbrüche, bricht der `Content-Disposition`-Header beim Multipart-Upload.
- **Verbesserungsvorschlag:**
  - In `UploadManager.cpp` `QString safeName = fileName; safeName.replace("\"", "_");` bereinigen.

---

## 3. Performance & Ressourceneffizienz

### PERF-01 (🔴 Kritisch): Massiv redundante Datei- und Exiv2-Zugriffe in `Photo` und `ImageIndexer`
- **Fundstellen:**
  - `src/ImageIndexer.cpp` (Zeilen 95–97):
    ```cpp
    Photo p(absPath);
    QString copyright = p.getCopyright();
    QString gps = p.getGpsDecimalString();
    ```
  - `src/rz_photo.cpp` (Zeilen 251–294, 366–426, 459–556)
- **Problem:**
  - Die Klasse `Photo` speichert nur den Dateipfad und öffnet bei **jedem einzelnen Methodenaufruf** die Bilddatei erneut über `Exiv2::ImageFactory::open(...)` und ruft `image->readMetadata()` bzw. `image->writeMetadata()` auf.
  - **Beim Ordnerscan (`ImageIndexer`):** Jedes Bild wird zweimal hintereinander komplett von Exiv2 geöffnet und geparst (1x für `getCopyright()`, 1x für `getGpsDecimalString()`). Bei 20.000 Bildern sind das **40.000 Datei-Öffnungen** statt 20.000.
  - **Beim Schreiben (`Photo::writeToAllCopyrightOwner()`):** Die Methode ruft nacheinander 3x `writeXmp()`, 2x `writeExif()` und 2x `writeIptc()` auf. Dadurch wird die Bilddatei auf der Festplatte **7-mal geöffnet, gelesen und komplett neu geschrieben**!
  - **Beim WebP-Export mit Zeitstempel (`Photo::convertImages()`):** `determineTimestamp()` wird in `convertImage()` für jede der 6 Zielgrößen erneut aufgerufen – das Quellbild wird also 6-mal mit Exiv2 geparst, um denselben Zeitstempel zu ermitteln, und 6-mal separat über `QImageReader` von der Festplatte dekodiert!
- **Verbesserungsvorschlag:**
  - In `Photo` eine Methode `readBasicSummary()` (oder internes Caching der gelesenen `Exiv2::Image::UniquePtr`-Metadaten) bereitstellen, die Copyright, GPS und Datum in **einem einzigen** `Exiv2::ImageFactory::open()`-Durchgang ausliest.
  - `writeToAllCopyrightOwner()` über `writeAllMetadata()` abbilden, sodass nur **ein einziger** Schreibvorgang (`image->writeMetadata()`) stattfindet.
  - In `Photo::convertImages()` das Quellbild (`QImage`) und den Zeitstempel **einmalig** im Speicher laden und daraus alle kleineren WebP-Auflösungen skalieren.

---

### PERF-02 (🟠 Hoch): Löschen der gesamten SQLite-Datenbank bei jedem Programmstart
- **Fundstelle:** `src/DatabaseManager.cpp` (Zeilen 57–64)
- **Problem:**
  ```cpp
  m_dbPath = dir.filePath("gallery_cache.db");
  if (QFile::exists(m_dbPath)) {
      if (QFile::remove(m_dbPath)) {
          qDebug() << "Old cache database deleted.";
      } ...
  }
  ```
  Obwohl die Datenbank in `QStandardPaths::CacheLocation` liegt und `CREATE TABLE IF NOT EXISTS` verwendet wird, wird die Datei `gallery_cache.db` bei jedem Start von `desktop-gallery` absichtlich gelöscht. Dadurch geht der gesamte Metadaten-Index verloren, und große Bildarchive müssen nach jedem Neustart komplett neu gescannt werden.
- **Verbesserungsvorschlag:**
  - Die Datenbank beim Start **beibehalten** und stattdessen in `ImageIndexer` inkrementell arbeiten (z. B. `last_modified`-Zeitstempel in der Tabelle speichern und nur neue oder geänderte Dateien mit Exiv2 parsen) sowie über eine Menü-Aktion „Cache leeren" (`DatabaseManager::clearDatabase()`) dem Benutzer bei Bedarf ein manuelles Zurücksetzen anbieten.

---

### PERF-03 (🟠 Hoch): Ungedrosselte `QImageReader`-Tasks in `ThumbnailDelegate::paint()`
- **Fundstelle:** `src/ThumbnailDelegate.cpp` (Zeilen 82–112)
- **Problem:**
  Solange ein Thumbnail noch von der Festplatte geladen wird (was bei großen Bildern 20–100 ms dauert), löst jedes Scroll-Event und jedes `viewport->update()` eines anderen fertiggestellten Thumbnails einen weiteren `paint()`-Aufruf für alle sichtbaren Kacheln aus. Ohne Deduplizierung werden für dasselbe Bild dutzende identische Dekodierungs-Tasks in den globalen `QThreadPool` eingereiht. Zudem ruft jeder fertige Task `viewport->update()` für den gesamten Viewport auf, statt nur das Rechteck des jeweiligen Index zu invalidieren.
- **Verbesserungsvorschlag:**
  - In-Flight-Set (`QSet<QString> m_pendingThumbnails`) zur Deduplizierung einführen.
  - `reader.setAutoTransform(true)` aktivieren, damit EXIF-Orientierungen auch in den Thumbnails korrekt dargestellt werden.

---

### PERF-04 (🟡 Mittel): Synchrone Bulk-Schreiboperationen im GUI-Thread & Einzel-SQL-Queries
- **Fundstelle:** `src/MainWindow.cpp` (Zeilen 565–680: `renameSelectedImages()`, `writeDefaultCopyrightToSelection()`, `writeDefaultGpsToSelection()`, `writeAllDefaultsToSelection()`)
- **Problem:**
  1. Alle vier Massenoperationen laufen synchron in einer `for`-Schleife auf dem GUI-Hauptthread. Bei Hunderten ausgewählten Bildern friert die Anwendung sekunden- oder minutenlang ein (das Betriebssystem markiert das Fenster als „Keine Rückmeldung").
  2. Für jedes geänderte Bild wird in `updateDbMetadata(path)` eine eigene SQLite-Transaktion (Auto-Commit pro `query.exec()`) ausgeführt. In SQLite kostet jedes Auto-Commit einen eigenen Disk-Sync.
- **Verbesserungsvorschlag:**
  - Die Datei-Schreiboperationen über `QtConcurrent` im Hintergrund ausführen (mit `QProgressDialog`).
  - Die anschließenden SQLite-Updates in einer einzigen Transaktion (`db.transaction()` ... `db.commit()`) bündeln.

---

### PERF-05 (🟡 Mittel): Speichern und wiederholtes Skalieren des vollauflösenden Bildes in `PictureWidget`
- **Fundstelle:** `src/picture_widget.cpp` (Zeilen 80, 89–91, 195–201)
- **Problem:**
  `picture = new QPixmap(pathToImage);` lädt das unskalierte Originalbild in den Grafikspeicher. Bei jedem `resizeEvent` des Fensters wird `picture->scaled(ui->picture_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)` auf dem vollen Originalbild im GUI-Thread berechnet, was das Vergrößern/Verkleinern des Detailfensters bei hochauflösenden Fotos (z. B. 45–60 MP) spürbar ruckeln lässt.
- **Verbesserungsvorschlag:**
  - Beim Laden `QImageReader` mit `setAutoTransform(true)` und einer sinnvollen Maximalauflösung (z. B. Bildschirmauflösung / 4K) verwenden oder beim `resizeEvent` zunächst `Qt::FastTransformation` nutzen und `Qt::SmoothTransformation` über einen kurzen Single-Shot-`QTimer` (z. B. 100 ms nach Ende der Größenänderung) verzögert nachziehen.

---

## 4. Empfohlene Priorisierung (Roadmap für Spec Kit Tasks)

1. **Priorität 1 – Kritische Stabilität & Performance (Sofortmaßnahmen):**
   - **STAB-01:** `PictureWidget::readSrcExif/Iptc/Xmp` entkoppeln (Exiv2-Read im Worker, `QTableWidget`-Update im GUI-Thread).
   - **STAB-02 & PERF-03:** `ThumbnailDelegate::paint()` mit `QPointer<QWidget>` und In-Flight-Deduplizierung (`QSet<QString>`) absichern.
   - **PERF-01:** `Photo`-Klasse um kombiniertes Metadaten-Lesen (`readSummary`) erweitern und `convertImages()` so optimieren, dass das Quellbild nur einmal geladen wird.
   - **STAB-04:** `PRAGMA journal_mode = MEMORY` aus `ImageIndexer.cpp` entfernen.
2. **Priorität 2 – Sicherheit & GUI-Reaktionsfähigkeit:**
   - **SEC-01:** HTML/JS-Escaping in `MapWindow.cpp` und `PictureWidget.cpp` einbauen.
   - **SEC-02:** `m_lastPass` aus `UploadManager` entfernen und HTTPS-Prüfung ergänzen.
   - **STAB-03 & STAB-05:** `processNextRegeocodeLookup()` und `exportSrcImgToWebpThread()` vollständig asynchronisieren (Iteration statt Rekursion, `QFutureWatcher` statt `future.result()`).
3. **Priorität 3 – Nachhaltige Optimierungen:**
   - **PERF-02:** Persistente SQLite-Cache-Datenbank mit Änderungszeitstempel (`file_mtime`) statt Löschen bei jedem Programmstart.
   - **PERF-04:** Bulk-Metadaten-Updates in SQLite-Transaktionen kapseln.
   - **SEC-03 & STAB-07:** `inifile-cpp` in `CMakeLists.txt` auf festen Tag/Commit pinnen, `QMenu`-Parenting in `PictureWidget` korrigieren.
