# Entwickler-Guide: SpecDD & GitHub Spec Kit

Willkommen im `desktop-gallery` (`web-gallery_desktop-gallery`) Projekt! Wir nutzen eine Kombination aus **SpecDD** (Architektur-Wahrheit) und **GitHub Spec Kit** (Feature-Entwicklung), um unsere **C++26**- und **Qt6**-Desktop-Anwendung robust, reaktionsschnell und wartbar zu halten und gleichzeitig zügig neue Funktionen zu entwickeln.
Dieser Guide erklärt dir, wie du als Entwickler oder KI-Agent damit im Alltag arbeitest.

## Warum zwei Frameworks?

- **SpecDD (`.sdd` Dateien & `docs/architecture/`):** Ist das „Grundgesetz" des Projekts. Es definiert die MVC-Architekturschichten (Presentation, Business Logic, Data Persistence), strikte C++26/Qt6-Threading- und Speichervorgaben, Datenbank-Invarianten sowie externe Schnittstellenverträge (CrowQtServer, Geocoding-Provider). Es ändert sich selten.
- **GitHub Spec Kit (Flow-Forward Spec):** Ist der strukturierte „Arbeitsauftrag". Wenn du etwas Neues bauen willst, nutzen wir einen iterativen, Agenten-gesteuerten Prozess (Plan -> Tasks -> Implement -> Converge), um das Feature sicher durch Header (`include/`), Implementierung (`src/`), UI-Formulare (`src/*.ui`), Übersetzungen (`i18n/`) und die `CMakeLists.txt` zu führen.

## Der Flow-Forward Workflow (Agentic SDD)

Da `desktop-gallery` mehrere Subsysteme innerhalb eines CMake-Projekts vereint (Qt6 Widgets/WebEngine UI, multithreaded `ImageIndexer`, `DatabaseManager` SQLite-Backend, `Photo` Exiv2-Wrapper, `regeocode`-Engine sowie `UploadManager` für das Crow Web-Gallery Ökosystem), orchestrieren wir Feature-Entwicklungen systematisch. Dies verhindert Architektur-Drift, blockierte GUI-Threads und fehlerhafte Builds.

Wir nutzen hierfür Agentic-Befehle (z. B. via Antigravity).

### 1. Rahmenbedingungen checken (SpecDD lesen)

Bevor du startest, verschaffe dir einen Überblick über die `.sdd`-Dateien (z. B. `desktop-gallery.sdd` im Root) und die Architekturdiagramme in `docs/architecture/`. Sie verraten dir, was du tun **musst** (z. B. teure Exiv2-/Dateioperationen aus dem UI-Thread auslagern, SPDX-MIT-Header verwenden, UI-Texte in `tr()` kapseln) und was **verboten** ist (z. B. direkter SQLite-Zugriff aus UI-Widgets unter Umgehung von `DatabaseManager` oder `QSqlTableModel`).

### 2. Spezifizieren & Planen (Design-Phase)

Bevor Code oder Specs geschrieben werden, wird durch den Agenten ein **Feature-Branch** (z. B. `feature/issue-42`) im Repository erstellt.

**Ablageort der Spec:**
- **Schichtübergreifende Features** (die UI, Business-Logik und Persistenz/Netzwerk betreffen) werden zentral als Spec-Verzeichnis unter `specs/features/<feature-name>/` angelegt.
- **Isolierte Subsystem-Features** (z. B. ein einzelner neuer Geocoding-Adapter in `regeocode`) können direkt im jeweiligen Modul-Spec-Ordner abgelegt werden (z. B. `specs/regeocode/<feature-name>/`).

- **`Run /speckit.specify`**: Der Agent (nachdem er den Branch erstellt hat) legt das neue Verzeichnis für dein Feature an und hilft dir, die initialen Anforderungen und Akzeptanzkriterien zu definieren.
- **`Run /speckit.plan`**: Der Agent analysiert die Anforderungen gegen die `.sdd`-Dateien und formuliert einen Architektur-Plan. Hier wird definiert, **wie** die betroffenen C++26/Qt6-Klassen und Signals/Slots miteinander interagieren (z. B. `MainWindow` startet asynchronen Task -> Worker verarbeitet Daten über `Photo` / `re_geocode_core` -> emittiert Signal -> aktualisiert `DatabaseManager` und UI).

### 3. Aufteilung & Vorbereitung (Work Breakdown)

- **`Run /speckit.tasks`**: Der Architektur-Plan wird in konkrete Teilaufgaben übersetzt. **Wichtig für C++26 & Qt6:** Die Tasks müssen explizit definieren, welche Dateien in welcher Reihenfolge geändert werden müssen:
  1. Header- & Interface-Verträge in `include/` (inklusive `Q_OBJECT`-Makros und Signal/Slot-Signaturen für MOC).
  2. Kernlogik-Implementierung in `src/` (bzw. `src/regeocode/`).
  3. UI-Formulare (`src/*.ui`), Qt-Ressourcen (`resources.qrc`) und Übersetzungen (`i18n/gallery_de.ts`, `i18n/gallery_en.ts`).
  4. Build-Konfiguration in `CMakeLists.txt` (Registrierung neuer `.cpp`-, `.hpp`- und `.ui`-Dateien).

### 4. Implementierung & Konvergenz (Die iterative Loop)

Dieser Schritt ersetzt den traditionellen, fehleranfälligen „Big Bang" Pull Request.

- **`Run /speckit.implement`**: Der Agent bearbeitet exakt den nächsten anstehenden Task. Er passt den C++26/Qt6-Code an, prüft die Kompilierung via CMake (`cmake --build build`) und präsentiert das Artefakt-Diff.
- **Review:** Du prüfst als Entwickler den Code-Diff dieses Einzelschritts.
- **`Run /speckit.converge`**: Der Agent analysiert den aktuellen Stand des Codes gegen die `specs/` und `.sdd`-Regeln. Sind alle SPDX-MIT-Header vorhanden? Werden die C++26-RAII-Regeln und Qt6-Thread-Grenzen eingehalten? Sind neue Quelldateien für `AUTOMOC`/`AUTOUIC` in der `CMakeLists.txt` eingetragen? Fehlt noch etwas, generiert er neue Folge-Tasks und fügt sie der Liste hinzu.
- **Repeat:** Wiederhole `/speckit.implement` und `/speckit.converge`, bis alle Tasks abgearbeitet sind und der Build sauber ohne Warnungen durchläuft.

### 5. Qualitätssicherung & Abschluss

- **Changelog & Architektur-Doku:** Im letzten Task der Implementierungs-Phase (oder in der finalen Converge-Loop) sorgt der Agent dafür, dass die `CHANGELOG.md` und – falls relevant – die Mermaid-Diagramme in `docs/architecture/` aktualisiert werden.
- **Push & PR:** Sobald die Converge-Loop erfolgreich ist, pusht der Agent (oder du) den Feature-Branch zu GitHub und eröffnet dort einen Pull Request (PR).
- CI/CD-Pipelines und Build-Skripte (`build_appimage.sh`, `create_dmg.sh`) prüfen abschließend die plattformübergreifende Build-Integrität.
- Sind alle Reviewer zufrieden, werden die Änderungen in den Hauptbranch (`main`) gemerged.

---

## 🛠️ Beispiele aus der Praxis

### Beispiel 1: Einen neuen Reverse-Geocoding-Adapter hinzufügen

Du möchtest, dass `desktop-gallery` neben Nominatim, Google, OpenCage und Bing einen weiteren Geocoding-Provider (z. B. Mapbox oder HERE) unterstützt.

1. **Plan & Specify:** Du nutzt `/speckit.specify` für `feature_mapbox_geocoder`. Der `/speckit.plan` erkennt durch `desktop-gallery.sdd` sofort: „Muss das `api_adapter`-Interface in `include/regeocode/` implementieren, Rate-Limits über den `QuotaManager` beachten und API-Keys aus der `re-geocode.ini` laden, ohne Secrets im Code zu hardcoden."
2. **Tasks:** `/speckit.tasks` zerlegt dies:
   - Task 1: Erstelle `include/regeocode/adapter_mapbox.hpp` und `src/regeocode/adapter_mapbox.cpp` als Implementierung von `api_adapter`.
   - Task 2: Registriere den Adapter in `re_geocode_core.cpp`, erweitere `data/re-geocode.ini_example` und trage die Dateien in die `CMakeLists.txt` ein.
3. **Implement:** Der Agent baut Schritt für Schritt den C++26-Adapter unter Nutzung von `http_client` und `nlohmann::json`, exakt gebunden an die Spec.

### Beispiel 2: Ein neues EXIF/IPTC/XMP-Metadatenfeld (Schichtübergreifend)

Du möchtest ein neues Metadaten-Attribut (z. B. Objektivmodell oder Aufnahmeort-Details) über alle Schichten hinweg auslesen, bearbeiten, synchronisieren und hochladen.

1. **Tasks:** `/speckit.tasks` generiert:
   - Task 1 (Metadata Layer): Erweitere `rz_metadata.hpp` und `rz_photo.cpp`, um das Tag via Exiv2 zwischen EXIF, IPTC und XMP zu lesen, zu schreiben und zu synchronisieren.
   - Task 2 (Persistence Layer): Passe `DatabaseManager.cpp` und `ImageIndexer.cpp` an, um das Attribut in SQLite zu indizieren und zu cachen.
   - Task 3 (Presentation & Cloud Layer): Binde das Feld in `picture_widget.ui` / `picture_widget.cpp` ein (inklusive `tr()`-Übersetzungen) und ergänze es im JSON-Payload von `UploadManager.cpp` für den `CrowQtServer`.
2. **Converge:** Wenn der Agent bei der Implementierung von Task 1 die Metadaten-Struktur anpasst, stellt `/speckit.converge` sofort sicher, dass die SQLite-Spaltenbindungen in Task 2 sowie die UI- und JSON-Bindungen in Task 3 konsistent mitgezogen werden.

---

Mit diesem Flow-Forward-Ansatz bleibt `desktop-gallery` über C++26, Qt6, SQLite und das Crow Web-Gallery Ökosystem hinweg jederzeit performant, threadsicher und architektonisch sauber!
