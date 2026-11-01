<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Developer Guide: SpecDD & GitHub Spec Kit](#developer-guide-specdd--github-spec-kit)
  - [Why Two Frameworks?](#why-two-frameworks)
  - [The Flow-Forward Workflow (Agentic SDD)](#the-flow-forward-workflow-agentic-sdd)
    - [1. Check Constraints (Read SpecDD)](#1-check-constraints-read-specdd)
    - [2. Specify & Plan (Design Phase)](#2-specify--plan-design-phase)
    - [3. Work Breakdown (Tasks)](#3-work-breakdown-tasks)
    - [4. Implementation & Converge (The Iterative Loop)](#4-implementation--converge-the-iterative-loop)
    - [5. Quality Assurance & Completion](#5-quality-assurance--completion)
  - [🛠️ Practical Examples](#-practical-examples)
    - [Example 1: Adding a New Reverse Geocoding Adapter](#example-1-adding-a-new-reverse-geocoding-adapter)
    - [Example 2: Adding a New EXIF/IPTC/XMP Metadata Field (Cross-Layer)](#example-2-adding-a-new-exifiptcxmp-metadata-field-cross-layer)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Developer Guide: SpecDD & GitHub Spec Kit

Welcome to the `desktop-gallery` (`web-gallery_desktop-gallery`) project! We use a combination of **SpecDD** (Architectural source of truth) and **GitHub Spec Kit** (Feature development) to keep our **C++26** and **Qt6** desktop application robust, responsive, and maintainable while rapidly building new features.
This guide explains how you, as a developer or AI agent, should work with them on a daily basis.

## Why Two Frameworks?

- **SpecDD (`.sdd` files & `docs/architecture/`):** This is the project's "constitution". It defines the MVC architectural layers (Presentation, Business Logic, Data Persistence), strict C++26/Qt6 threading and memory rules, database schema invariants, and external API contracts (CrowQtServer, Geocoding providers). It rarely changes.
- **GitHub Spec Kit (Flow-Forward Spec):** This is the structured "work order". When building something new, we use an iterative, agent-driven process (Plan -> Tasks -> Implement -> Converge) to safely guide the feature across headers (`include/`), implementation (`src/`), UI forms (`src/*.ui`), translations (`i18n/`), and `CMakeLists.txt`.

## The Flow-Forward Workflow (Agentic SDD)

Since `desktop-gallery` integrates multiple subsystems within a single CMake project (Qt6 Widgets/WebEngine UI, multithreaded `ImageIndexer`, `DatabaseManager` SQLite backend, `Photo` Exiv2 wrapper, `regeocode` engine, and `UploadManager` for the Crow Web-Gallery ecosystem), we orchestrate feature development systematically. This prevents architectural drift, blocked GUI threads, and broken builds.

We use Agentic commands (e.g., via Antigravity) for this.

### 1. Check Constraints (Read SpecDD)

Before starting, review the `.sdd` files (e.g., `desktop-gallery.sdd` in the root) and the architecture diagrams in `docs/architecture/`. They tell you what you **must** do (e.g., keep heavy Exiv2/disk operations off the UI thread, use SPDX MIT headers, wrap UI strings in `tr()`) and what is **forbidden** (e.g., direct SQLite access from UI widgets bypassing `DatabaseManager` or `QSqlTableModel`).

### 2. Specify & Plan (Design Phase)

Before any code or specs are written, the agent creates a **feature branch** (e.g., `feature/issue-42`) in the repository.

**Spec Location:**
- **Cross-layer features** (affecting UI, Business Logic, and Persistence/Network) are created centrally as a spec directory under `specs/features/<feature-name>/`.
- **Isolated subsystem features** (e.g., a single new geocoding adapter in `regeocode`) can be scoped directly to their module spec folder (e.g., `specs/regeocode/<feature-name>/`).

- **`Run /speckit.specify`**: The agent (after creating the branch) creates the new directory for your feature and helps you define the initial requirements and acceptance criteria.
- **`Run /speckit.plan`**: The agent analyzes the requirements against the `.sdd` files and formulates an architectural plan. This defines **how** the affected C++26/Qt6 classes and Signals/Slots will interact (e.g., `MainWindow` triggers asynchronous task -> Worker processes via `Photo` / `re_geocode_core` -> emits signal -> updates `DatabaseManager` and UI).

### 3. Work Breakdown (Tasks)

- **`Run /speckit.tasks`**: The architectural plan is translated into specific subtasks. **Crucial for C++26 & Qt6:** The tasks must explicitly define which files need to be changed in which order:
  1. Header & interface contracts in `include/` (including `Q_OBJECT` macros and Signal/Slot signatures for MOC).
  2. Core logic implementation in `src/` (and `src/regeocode/`).
  3. UI forms (`src/*.ui`), Qt resources (`resources.qrc`), and translations (`i18n/gallery_de.ts`, `i18n/gallery_en.ts`).
  4. Build configuration in `CMakeLists.txt` (registering new `.cpp`, `.hpp`, and `.ui` files).

### 4. Implementation & Converge (The Iterative Loop)

This step replaces the traditional, error-prone "Big Bang" pull request.

- **`Run /speckit.implement`**: The agent works on the next pending task. It modifies the C++26/Qt6 code, verifies compilation via CMake (`cmake --build build`), and presents the artifact diff.
- **Review:** As a developer, you review the code diff for this single step.
- **`Run /speckit.converge`**: The agent analyzes the current state of the code against the `specs/` and `.sdd` rules. Are all SPDX MIT headers present? Are C++26 memory/RAII rules and Qt6 thread boundaries respected? Are new sources added to `CMakeLists.txt` for `AUTOMOC`/`AUTOUIC`? If anything is missing, it generates follow-up tasks and appends them to the list.
- **Repeat:** Repeat `/speckit.implement` and `/speckit.converge` until all tasks are completed and the build succeeds cleanly without warnings.

### 5. Quality Assurance & Completion

- **Changelog & Architecture Docs:** In the final task of the implementation phase (or in the final converge loop), the agent ensures that `CHANGELOG.md` and, if applicable, the Mermaid diagrams in `docs/architecture/` are updated.
- **Push & PR:** Once the converge loop is successful, the agent (or you) pushes the feature branch to GitHub and opens a Pull Request (PR).
- CI/CD pipelines and build scripts (`build_appimage.sh`, `create_dmg.sh`) verify cross-platform build integrity.
- If all reviewers are satisfied, the changes are merged into `main`.

---

## 🛠️ Practical Examples

### Example 1: Adding a New Reverse Geocoding Adapter

You want `desktop-gallery` to support a new geocoding provider (e.g., Mapbox or HERE) alongside Nominatim, Google, OpenCage, and Bing.

1. **Plan & Specify:** You use `/speckit.specify` for `feature_mapbox_geocoder`. Through `desktop-gallery.sdd`, `/speckit.plan` immediately recognizes: "Must implement `api_adapter` interface in `include/regeocode/`, respect `QuotaManager` rate limits, and load API keys from `re-geocode.ini` without hardcoding secrets."
2. **Tasks:** `/speckit.tasks` breaks this down:
   - Task 1: Create `include/regeocode/adapter_mapbox.hpp` and `src/regeocode/adapter_mapbox.cpp` implementing `api_adapter`.
   - Task 2: Register the adapter in `re_geocode_core.cpp`, update `data/re-geocode.ini_example`, and add files to `CMakeLists.txt`.
3. **Implement:** The agent incrementally builds the C++26 adapter using `http_client` and `nlohmann::json`, strictly adhering to the spec.

### Example 2: Adding a New EXIF/IPTC/XMP Metadata Field (Cross-Layer)

You want to read, edit, synchronize, and upload a new metadata attribute (e.g., Lens Model or Photographer Location) across the application.

1. **Tasks:** `/speckit.tasks` generates:
   - Task 1 (Metadata Layer): Extend `rz_metadata.hpp` and `rz_photo.cpp` to read/write and sync the tag across EXIF, IPTC, and XMP via Exiv2.
   - Task 2 (Persistence Layer): Update `DatabaseManager.cpp` and `ImageIndexer.cpp` to index and cache the attribute in SQLite.
   - Task 3 (Presentation & Cloud Layer): Expose the field in `picture_widget.ui` / `picture_widget.cpp` (with `tr()` i18n entries) and include it in `UploadManager.cpp` JSON payloads for `CrowQtServer`.
2. **Converge:** When the agent adapts the metadata struct in Task 1, `/speckit.converge` ensures that SQLite column bindings in Task 2 and UI/JSON bindings in Task 3 stay completely synchronized.

---

With this Flow-Forward approach, `desktop-gallery` remains fast, thread-safe, and architecturally clean across C++26, Qt6, SQLite, and the Crow Web-Gallery ecosystem!
