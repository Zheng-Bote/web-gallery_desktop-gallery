# Integrationsmatrix: SpecDD × GitHub Spec Kit für Desktop-Gallery

Dieses Dokument definiert, wie im `desktop-gallery` (`web-gallery_desktop-gallery`) Projekt – entwickelt mit **C++26** und **Qt6** – die Architekturspezifikation (SpecDD) und die Featurespezifikation (GitHub Spec Kit) zusammenarbeiten.

- **SpecDD** → Architektur-First, System-Intent, zentrale Schichten & Kernmodule (Presentation/UI Layer, Business Logic Layer, Data Persistence Layer, Multithreading/Concurrency, Metadaten- & Geocoding-Engine).
- **Spec Kit** → Feature-First, schnelle Iteration, isolierte Erweiterungen (z. B. ein neuer Geocoding-Adapter, eine neue EXIF/IPTC/XMP-Mapping-Regel, UI-Dialog-Anpassungen oder Export-Filter).

## 1. System-Ebene (SpecDD) vs. Feature-Ebene (Spec Kit)

| Ebene                       | Artefakt                                            | Werkzeug | Zweck                                                              |
| --------------------------- | --------------------------------------------------- | -------- | ------------------------------------------------------------------ |
| **System-Intent**           | `.sdd` Specs (`desktop-gallery.sdd` & Modul-Level)  | SpecDD   | MVC-Architektur, C++26/Qt6-Richtlinien, Threading, Security        |
| **System-Architektur**      | `README.md`, `docs/architecture/*.md`               | SpecDD   | Signal/Slot-Orchestrierung, SQLite-Schema, Exiv2-Wrapper, REST     |
| **System-Governance**       | `.sdd` (Must / Must not / Forbids)                  | SpecDD   | Nicht-Blockieren des GUI-Threads, Credential-Schutz, Memory/RAII   |
| **Feature-Intent**          | Feature-Spec (Markdown)                             | SpecKit  | Isolierte Änderungen (z. B. neuer Geocoding-Adapter, Wasserzeichen)|
| **Feature-Planung**         | Plan-Dokument / Issue                               | SpecKit  | Tasks, Scope, Acceptance Criteria                                  |
| **Feature-Implementierung** | Tasks                                               | SpecKit  | AI-freundliche Umsetzung über `include/`, `src/`, `i18n/`          |
| **Feature-Änderungen**      | Deltas                                              | SpecKit  | Versionierung einzelner Features                                   |

## 2. Integrationslogik (Wer überschreibt wen?)

| Bereich                      | Quelle  | Regel                                                                                                      |
| ---------------------------- | ------- | ---------------------------------------------------------------------------------------------------------- |
| **Architektur & Threading**  | SpecDD  | SpecDD ist _Single Source of Truth_ (MVC-Schichtung, asynchrone `QtConcurrent`-Worker).                    |
| **Security & Credentials**   | SpecDD  | SpecKit darf keine API-Keys hardcoden oder JWT-Auth / Prepared SQL-Statements umgehen.                     |
| **Geocoding (`regeocode`)**  | SpecKit | SpecDD definiert das `api_adapter`-Interface & `QuotaManager`, SpecKit liefert konkrete API-Adapter.       |
| **Metadaten (`rz_photo`)**   | SpecKit | SpecDD definiert den Exiv2-Wrapper & die Sync-Architektur, SpecKit liefert Tag-Mappings und Bulk-Regeln.   |
| **Cloud-Upload & Export**    | SpecKit | SpecDD definiert `UploadManager` & JWT-Lifecycle, SpecKit definiert spezifische Payload-/Retry-Features.   |
| **Persistenz (`QtSql`)**     | SpecDD  | SpecKit darf Spalten/Tabellen im `DatabaseManager` für Features ergänzen, aber das Kernschema nicht brechen.|

## 3. Synchronisationsregeln (Drift vermeiden)

| Situation                       | Aktion                                     | Werkzeug |
| ------------------------------- | ------------------------------------------ | -------- |
| Neues Feature (z. B. Adapter/UI)| SpecKit-Spec → Tasks → Implement           | SpecKit  |
| Feature beeinflusst Architektur | SpecKit-Feature erzwingt Update der `.sdd` | SpecDD   |
| Architekturänderung             | SpecDD → neue SpecKit-Features ableiten    | SpecKit  |
| Konflikt zwischen Specs         | SpecDD gewinnt                             | SpecDD   |
| Neue Normen (z. B. C++26 / Qt6) | SpecDD aktualisieren → SpecKit anpassen    | Beide    |

## 4. Artefakt-Mapping für Desktop-Gallery

| Desktop-Gallery Bereich | SpecDD                                             | Spec Kit                                                    |
| ----------------------- | -------------------------------------------------- | ----------------------------------------------------------- |
| **Presentation (UI)**   | ✔ MVC-View-Design, Qt6 Widgets/WebEngine, i18n     | ✔ Neue Dialoge, `PictureWidget`-Felder, Karten-Marker       |
| **Metadaten & Bild**    | ✔ `Photo` Exiv2-Wrapper, EXIF/IPTC/XMP-Sync-Kern   | ✔ Spezifische Tag-Mappings, WebP-Export-Optionen, Watermarks|
| **Geocoding**           | ✔ `re_geocode_core`, Adapter-Pattern, Quota-Logik  | ✔ Neue Geocoding-Provider (Nominatim, OpenCage etc.)        |
| **Cloud-Upload**        | ✔ `UploadManager` State-Machine, CrowQtServer-API  | ✔ Multipart-Payload-Anpassungen, Upload-Progress-UI         |
| **Storage & Indexing**  | ✔ `DatabaseManager` (SQLite3), `ImageIndexer`      | ✔ Benutzerdefinierte Suchfilter, Feature-spezifische Queries|
| **Build & Packaging**   | ✔ CMake (C++26/Qt6), AppImage & macOS DMG          | ✔ Einbinden neuer Feature-Sources/Header & `.ts`-Dateien    |

## 5. Integrationsmatrix

| Kategorie               | SpecDD | Spec Kit |
| ----------------------- | ------ | -------- |
| System-Intent           | ✔      | ✖        |
| Architektur             | ✔      | ✖        |
| Governance / Security   | ✔      | ✖        |
| Feature-Intent          | ✖      | ✔        |
| Feature-Planung         | ✖      | ✔        |
| Feature-Implementierung | ✖      | ✔        |
| Drift-Kontrolle         | ✔      | ✔        |

## 6. Code-Qualität & Standard-Baseline

Alle Spec Kit Features MÜSSEN die strikten Basisstandards einhalten, die in der `desktop-gallery.sdd` und `GEMINI.md` definiert sind. KI-Agenten und Entwickler dürfen keinen Code generieren, der diese verletzt:
- **Sprache & i18n:** Alle Code-Dokumentationen, Doxygen-Kommentare und Inline-Kommentare müssen auf Englisch sein. Alle sichtbaren UI-Texte müssen im Qt-Makro `tr()` gekapselt und in `i18n/gallery_de.ts` sowie `i18n/gallery_en.ts` gepflegt werden.
- **Lizenzierung:** Jede Quell- (`.cpp`) und Header-Datei (`.hpp`/`.h`) muss den standardisierten SPDX- & Doxygen-Header (C++-Style Blockkommentar) mit `SPDX-License-Identifier: MIT` (`Copyright (c) 2026 ZHENG Robert`) enthalten.
- **C++26 & Qt6 Architektur:** Der Code muss moderne C++26-Idiome nutzen (RAII, Smart Pointer, Value Semantics, starke Typisierung) sowie die Qt6 Parent-Child-Ownership und MOC-Anforderungen beachten (Header mit `Q_OBJECT` müssen in der `CMakeLists.txt` registriert sein). Rechen- oder I/O-intensive Operationen (Bild-Indizierung, Exiv2-Schreibzugriffe, Netzwerkaufrufe) dürfen niemals den Qt-GUI-Hauptthread blockieren.
- **Security & Secrets:** API-Keys (z. B. in `re-geocode.ini`) und CrowQtServer-JWT-Tokens dürfen niemals im Quellcode hinterlegt oder ins Git-Repository committet werden. Alle SQLite-Abfragen im `DatabaseManager` müssen Prepared Statements (`QSqlQuery::prepare`) verwenden.
