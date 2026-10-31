<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Feature Intent](#feature-intent)
- [Scope](#scope)
- [SpecDD Architecture Alignment (Drift Control)](#specdd-architecture-alignment-drift-control)
- [Acceptance Criteria](#acceptance-criteria)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

---
name: Feature Specification (Spec Kit)
about: Propose a new feature, geocoding adapter, metadata mapping rule, or UI enhancement.
title: 'Feature: '
labels: 'feature, spec-kit'
assignees: ''
---

## Feature Intent
*Describe what the feature does and why it is needed. Keep it concise.*

## Scope
*What components (UI / Widgets, ImageIndexer, Photo/Exiv2, Regeocode, UploadManager, DatabaseManager) does this affect?*

## SpecDD Architecture Alignment (Drift Control)
Please confirm that this feature respects the global `desktop-gallery` constraints defined in `.sdd` files:
- [ ] **Architecture & Threading:** MVC layering is maintained and the Qt GUI thread is never blocked by heavy I/O or network calls.
- [ ] **Security:** API keys and JWT tokens are not hardcoded; SQLite queries use prepared statements (`QSqlQuery::prepare`).
- [ ] **Data Model:** Core SQLite schemas in `DatabaseManager` remain intact, and EXIF/IPTC/XMP synchronization via `Photo` is preserved.
- [ ] **Standards:** SPDX MIT headers, English code comments, `tr()` i18n support, and C++26 (Linux) / C++23 (Windows) compatibility are maintained.

## Acceptance Criteria
- [ ] Criterion 1
- [ ] Criterion 2
- [ ] CHANGELOG.md and README.md are up-to-date
