<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Integration Matrix: SpecDD × GitHub Spec Kit for Desktop-Gallery](#integration-matrix-specdd-%C3%97-github-spec-kit-for-desktop-gallery)
  - [1. System Level (SpecDD) vs. Feature Level (Spec Kit)](#1-system-level-specdd-vs-feature-level-spec-kit)
  - [2. Integration Logic (Who Overrides Whom?)](#2-integration-logic-who-overrides-whom)
  - [3. Synchronization Rules (Avoiding Drift)](#3-synchronization-rules-avoiding-drift)
  - [4. Artifact Mapping for Desktop-Gallery](#4-artifact-mapping-for-desktop-gallery)
  - [5. Integration Matrix](#5-integration-matrix)
  - [6. Code Quality & Standards Baseline](#6-code-quality--standards-baseline)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Integration Matrix: SpecDD × GitHub Spec Kit for Desktop-Gallery

This document defines how architecture specification (SpecDD) and feature specification (GitHub Spec Kit) work together in the `desktop-gallery` (`web-gallery_desktop-gallery`) project built with **C++26** and **Qt6**.

- **SpecDD** → Architecture-First, System-Intent, core layers & modules (Presentation/UI Layer, Business Logic Layer, Data Persistence Layer, Multithreading/Concurrency, Metadata & Geocoding Engine).
- **Spec Kit** → Feature-First, fast iteration, isolated enhancements (e.g., a new geocoding adapter, a new EXIF/IPTC/XMP mapping rule, UI dialog refinements, or export filters).

## 1. System Level (SpecDD) vs. Feature Level (Spec Kit)

| Level                       | Artifact                                            | Tool     | Purpose                                                        |
| --------------------------- | --------------------------------------------------- | -------- | -------------------------------------------------------------- |
| **System Intent**           | `.sdd` Specs (`desktop-gallery.sdd` & Module Level) | SpecDD   | MVC Architecture, C++26/Qt6 Guidelines, Threading, Security    |
| **System Architecture**     | `README.md`, `docs/architecture/*.md`               | SpecDD   | Signal/Slot Orchestration, SQLite Schema, Exiv2 Wrapper, REST  |
| **System Governance**       | `.sdd` (Must / Must not / Forbids)                  | SpecDD   | GUI-Thread Non-Blocking Rules, Credential Handling, Memory/RAII|
| **Feature Intent**          | Feature Spec (Markdown)                             | SpecKit  | Isolated changes (e.g., new Geocoding Adapter, WebP watermark) |
| **Feature Planning**        | Plan Document / Issue                               | SpecKit  | Tasks, Scope, Acceptance Criteria                              |
| **Feature Implementation**  | Tasks                                               | SpecKit  | AI-friendly implementation across `include/`, `src/`, `i18n/`  |
| **Feature Changes**         | Deltas                                              | SpecKit  | Versioning of individual features                              |

## 2. Integration Logic (Who Overrides Whom?)

| Area                         | Source  | Rule                                                                                              |
| ---------------------------- | ------- | ------------------------------------------------------------------------------------------------- |
| **Architecture & Threading** | SpecDD  | SpecDD is the _Single Source of Truth_ (MVC layering, asynchronous `QtConcurrent` workers).       |
| **Security & Credentials**   | SpecDD  | SpecKit must not hardcode API keys or bypass JWT authentication / prepared SQL statements.        |
| **Geocoding (`regeocode`)**  | SpecKit | SpecDD defines `api_adapter` interface & `QuotaManager`, SpecKit provides specific API adapters.  |
| **Metadata (`rz_photo`)**    | SpecKit | SpecDD defines the Exiv2 wrapper & sync architecture, SpecKit provides tag mappings/bulk rules.   |
| **Cloud Upload & Export**    | SpecKit | SpecDD defines `UploadManager` & JWT lifecycle, SpecKit defines specific payload/retry features.  |
| **Persistence (`QtSql`)**    | SpecDD  | SpecKit may add columns/tables for features in `DatabaseManager`, but must not break core schema. |

## 3. Synchronization Rules (Avoiding Drift)

| Situation                       | Action                                  | Tool     |
| ------------------------------- | --------------------------------------- | -------- |
| New Feature (e.g., Adapter/UI)  | SpecKit Spec → Tasks → Implement        | SpecKit  |
| Feature affects Architecture    | SpecKit feature forces `.sdd` update    | SpecDD   |
| Architecture Change             | SpecDD → derive new SpecKit features    | SpecKit  |
| Conflict between Specs          | SpecDD wins                             | SpecDD   |
| New Norms (e.g., C++26 / Qt6)   | Update SpecDD → adjust SpecKit          | Both     |

## 4. Artifact Mapping for Desktop-Gallery

| Desktop-Gallery Area   | SpecDD                                            | Spec Kit                                                 |
| ---------------------- | ------------------------------------------------- | -------------------------------------------------------- |
| **Presentation (UI)**  | ✔ MVC View Design, Qt6 Widgets/WebEngine, i18n    | ✔ New Dialogs, `PictureWidget` Fields, Map Markers       |
| **Metadata & Image**   | ✔ `Photo` Exiv2 Wrapper, EXIF/IPTC/XMP Sync Core  | ✔ Specific Tag Mappings, WebP Export Options, Watermarks |
| **Geocoding**          | ✔ `re_geocode_core`, Adapter Pattern, Quota Logic | ✔ New Geocoding Providers (Nominatim, OpenCage, etc.)    |
| **Cloud Upload**       | ✔ `UploadManager` State Machine, CrowQtServer API | ✔ Multipart Payload Adjustments, Upload Progress UI      |
| **Storage & Indexing** | ✔ `DatabaseManager` (SQLite3), `ImageIndexer`     | ✔ Custom Search Filters, Feature-Specific DB Queries     |
| **Build & Packaging**  | ✔ CMake (C++26/Qt6), AppImage & macOS DMG         | ✔ Adding Feature Sources/Headers & `.ts` Translations    |

## 5. Integration Matrix

| Category                | SpecDD | Spec Kit |
| ----------------------- | ------ | -------- |
| System Intent           | ✔      | ✖        |
| Architecture            | ✔      | ✖        |
| Governance / Security   | ✔      | ✖        |
| Feature Intent          | ✖      | ✔        |
| Feature Planning        | ✖      | ✔        |
| Feature Implementation  | ✖      | ✔        |
| Drift Control           | ✔      | ✔        |

## 6. Code Quality & Standards Baseline

All Spec Kit features MUST adhere to the strict baseline standards defined in `desktop-gallery.sdd` and `GEMINI.md`. AI agents and developers must not generate code that violates these:
- **Language & i18n:** All source code documentation, Doxygen comments, and inline comments must be in English. All user-facing UI strings must be wrapped in Qt's `tr()` macro and updated in `i18n/gallery_de.ts` and `i18n/gallery_en.ts`.
- **Licensing:** Every source (`.cpp`) and header (`.hpp`/`.h`) file must include the standardized SPDX & Doxygen header (C++ style block comment) with `SPDX-License-Identifier: MIT` (`Copyright (c) 2026 ZHENG Robert`).
- **C++26 & Qt6 Architecture:** Code must use modern C++26 idioms (RAII, smart pointers, value semantics, strong types) and respect Qt6 parent-child ownership and MOC requirements (headers with `Q_OBJECT` registered in `CMakeLists.txt`). Heavy I/O (image indexing, Exiv2 metadata writes, network calls) must never block the Qt GUI main thread.
- **Security & Secrets:** API keys (e.g., in `re-geocode.ini`) and CrowQtServer JWT tokens must never be hardcoded in source code or committed to version control. All SQLite queries in `DatabaseManager` must use bound parameters (`QSqlQuery::prepare`) to guarantee safety and integrity.
