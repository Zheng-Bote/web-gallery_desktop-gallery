<!-- DOCTOC SKIP -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to Semantic Versioning.

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
