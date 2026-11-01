<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Feature: [Feature Name]](#feature-feature-name)
  - [1. Feature Intent](#1-feature-intent)
  - [2. Affected Components](#2-affected-components)
  - [3. SpecDD Constraints & Architecture Impact](#3-specdd-constraints--architecture-impact)
  - [4. Implementation Tasks (AI-Friendly)](#4-implementation-tasks-ai-friendly)
  - [5. Acceptance Criteria](#5-acceptance-criteria)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Feature: [Feature Name]

**Status:** Draft | In Progress | Done
**Author:** [Name]

## 1. Feature Intent
*What is the goal of this feature? What value does it add to `desktop-gallery`?*

## 2. Affected Components
*List the layers and modules (Presentation/UI, Business Logic / `ImageIndexer` / `Photo` / `regeocode` / `UploadManager`, Persistence / `DatabaseManager`) that will be modified.*

## 3. SpecDD Constraints & Architecture Impact
*Describe how this feature integrates with the existing SpecDD architecture. Explicitly address:*
- **Threading & Responsiveness:** Does it perform disk I/O, Exiv2 parsing, or network calls? How is it offloaded from the Qt GUI thread (`QtConcurrent` / `QtNetwork`)?
- **Persistence:** Does it require new SQLite columns/tables or prepared queries in `DatabaseManager`?
- **Security & Config:** Does it handle API keys (`re-geocode.ini`), rate limits (`QuotaManager`), or JWT auth (`UploadManager`)?
- **UI & i18n:** Does it add user-facing strings requiring `tr()` and updates in `i18n/gallery_de.ts` and `i18n/gallery_en.ts`?

## 4. Implementation Tasks (AI-Friendly)
*List concrete, actionable tasks for implementation.*
- [ ] Task 1: Define header/interface contracts in `include/` (with SPDX MIT header and `Q_OBJECT` if needed).
- [ ] Task 2: Implement core C++26 / Qt6 logic in `src/` with English Doxygen/inline comments.
- [ ] Task 3: Update UI forms (`src/*.ui`), translations (`i18n/*.ts`), and register new files in `CMakeLists.txt`.

## 5. Acceptance Criteria
*How do we know this is done?*
- [ ] Compiles cleanly with CMake (`cmake --build build`).
- [ ] Respects all constraints in `desktop-gallery.sdd`.
- [ ] ...
