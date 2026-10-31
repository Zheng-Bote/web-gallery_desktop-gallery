<!-- DOCTOC SKIP -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to Semantic Versioning.

## [1.2.0] - 2026-10-31

### Added

- **AVIF Export (Quality 75):** Added single-size and multi-size (`Original`, `480`, `680`/`640`, `800`, `1024`, `1280`) AVIF export in both `MainWindow` (`AVIF Export` menu and context menu) and `PictureWidget` (`export to AVIF` toolbar menu) using `QImageWriter("avif")` with automatic fallback to `Magick++` (ImageMagick 7).
- **Persistent SQLite Metadata Cache (`PERF-02`):** Retained `gallery_cache.db` across restarts with incremental `file_mtime` / `file_size` checks in `ImageIndexer` and an explicit **Clear Database Cache...** action in the Settings menu.
- **SpecDD & GitHub Spec Kit Integration:** Added `.specdd/` bootstrap configuration, `desktop-gallery.sdd`, `AGENTS.md`, `GEMINI.md`, `CLAUDE.md`, `.github/spec-kit/`, and feature specifications under `specs/features/`.
- **German Translations (`i18n`):** Added German translations for AVIF export actions and dialogs.

### Changed

- **C++ Standard:** Upgraded CMake build configuration (`cmake_minimum_required(VERSION 3.25)`) to **C++26** on Linux and **C++23** on Windows/macOS.
- **Single-Pass Exiv2 & Image I/O (`PERF-01`):** Implemented `Photo::readSummary()`, consolidated `Photo::writeToAllCopyrightOwner()` into a single write pass, and decoded source images once per multi-size WebP/AVIF export.
- **Asynchronous Bulk Operations (`PERF-04`, `STAB-03`, `STAB-05`):** Moved batch reverse geocoding (`processNextRegeocodeLookup`), bulk metadata writes, bulk timestamp renaming, and `PictureWidget` WebP/AVIF exports off the GUI thread using `QtConcurrent::run` and batched SQLite transactions (`db.transaction()` / `db.commit()`).
- **Smooth Viewer Resizing (`PERF-05`):** Updated `PictureWidget::resizeEvent()` to use `Qt::FastTransformation` during interactive resizing and debounced `Qt::SmoothTransformation` via a 100 ms single-shot `QTimer`.

### Fixed

- **Thread-Safety & Stability (`STAB-01`, `STAB-02`, `STAB-04`, `STAB-06`, `STAB-07`):**
  - Decoupled background Exiv2 metadata reading from GUI-thread `QTableWidget` population in `PictureWidget`.
  - Added `QPointer` lifetime guards, EXIF auto-orientation (`setAutoTransform(true)`), and in-flight deduplication (`m_loadingCacheKeys`) in `ThumbnailDelegate`.
  - Removed unsafe `PRAGMA journal_mode = MEMORY` / `synchronous = OFF` overrides in `ImageIndexer`, added `PRAGMA busy_timeout = 5000`, and added cooperative cancellation on re-scan/shutdown.
  - Replaced `std::localtime` with thread-safe `localtime_r` / `localtime_s` in `QuotaManager` and initialized `curl_global_init()` deterministically in `main()`.
  - Fixed `QMenu` and `QPixmap` ownership in `PictureWidget`.
- **Security Hardening (`SEC-01`, `SEC-02`, `SEC-03`, `SEC-04`):**
  - Escaped filenames and serialized marker payloads via `QJsonDocument` in `MapWindow` and `PictureWidget` to prevent DOM-XSS in `QWebEngineView`.
  - Removed plaintext credential caching in `UploadManager`, sanitized multipart `Content-Disposition` filenames, and added an unencrypted HTTP warning before non-localhost uploads.
  - Pinned `inifile-cpp` to an immutable commit hash in `CMakeLists.txt` and added Subresource Integrity (`integrity` / `crossorigin`) attributes for Leaflet 1.9.4 CDN assets.

## [1.1.0] - 2026-03-08

### Added

- Integrated regeocode libraries for geocoding and timezone lookup
- Menu item to set the path to desktop-gallery_regeocode.ini
- User prompt and API selection in the menu
- Saving address_english, address_local, country_code, timezone_id, local_time, gmt_offset as XMP metadata in the image

### Fixed

- Synchronization of function signatures and bugfixes in implementation

### Changed

- Project version updated to 1.1.0

## [Unreleased]

### Changed

- The API strategy for regeocode lookups is now dynamically loaded from the INI configuration ([strategies]->default) and no longer hardcoded.
- The quota file ([config]->quota-file) is correctly loaded from the INI and passed to the QuotaManager.
- Bugfixes: More robust INI parsing logic, correct type conversion and section checking.

### Fixed

- Compilation errors due to incorrect method calls and type conversions in INI parsing resolved.

## [0.2.0] - 2026-03-07

### Added

- **SDPX Headers:** Added missing MIT License SPDX headers to all C++ source (`.cpp`) and header (`.hpp`) files in `src/` and `include/`.
- **Doxygen Documentation:** Standardized file-level comments in all C++ files over to professional Doxygen English comments, replacing older German and English comments.
- **Architecture Diagrams:** Added a comprehensive suite of architectural diagrams (Mermaid) to `docs/architecture/`, including:
  - Bounded Contexts, Class, Sequence, Activity, State, Component, Deployment, Use Case, Entity Relationship, Flowcharts, Information Flow, and Decomposition diagrams.
- **macOS Installer:** Added a `create_dmg.sh` script to automatically generate a drag-and-drop `.dmg` installer for macOS using `macdeployqt` and `CPack`.
- **macOS Native Integration:** App bundle is now ad-hoc signed to prevent "Killed: 9" security crashes on ARM64, and native macOS menus are properly initialized with English fallbacks to prevent them from being hidden by the OS.
- **CI/CD:** Added GitHub Actions workflow (`quality_security.yml`) for automated C++ Static Analysis (CodeQL, Cppcheck, Clang-Tidy) and Security/SBOM scanning (CycloneDX, cve-bin-tool).
- **CHANGELOG:** Introduced standard `CHANGELOG.md` file to track project history.

### Changed

- **README:** Updated `README.md` to reference the newly generated architectural documentation.
