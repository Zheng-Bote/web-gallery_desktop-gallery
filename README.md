<div id="top" align="center">
<h1>Desktop-Gallery (Qt6 / C++23)</h1>

<p>A high-performance, professional Desktop Frontend (for the Crow Web-Gallery Project).</p>
<p>Developed for Linux, macOS, and Windows.</p>

[Report Issue](https://github.com/Zheng-Bote/qt_desktop-gallery/issues) · [Request Feature](https://github.com/Zheng-Bote/qt_desktop-gallery/pulls)

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)
[![Qt6](https://img.shields.io/badge/Qt-6.9-41CD52?logo=qt)](https://www.qt.io/)
[![C++23](https://img.shields.io/badge/C++-23-blue?logo=cplusplus)](https://en.cppreference.com/w/cpp/23)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Zheng-Bote/qt_desktop-gallery?logo=GitHub)](https://github.com/Zheng-Bote/qt_desktop-gallery/releases)

</div>

<hr>
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Description](#description)
  - [Key Features](#key-features)
    - [🚀 Performance \& Core](#-performance--core)
    - [🌍 Geo-Intelligence](#-geo-intelligence)
    - [📝 Metadata Management](#-metadata-management)
    - [☁️ Export \& Cloud](#️-export--cloud)
  - [see also](#see-also)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build Instructions](#build-instructions)
    - [Usage Workflow](#usage-workflow)
      - [Import](#import)
      - [Organize](#organize)
      - [Edit Metadata](#edit-metadata)
      - [Geocode](#geocode)
      - [Visualize](#visualize)
      - [Export/Upload](#exportupload)
- [Documentation \& Screenshots](#documentation--screenshots)
  - [Album View](#album-view)
  - [Metadata Editor \& Sync](#metadata-editor--sync)
  - [Context Menu](#context-menu)
  - [Map View](#map-view)
  - [WebP Export](#webp-export)
  - [Internationalization (i18n)](#internationalization-i18n)
  - [Detail View](#detail-view)
  - [Project Structure](#project-structure)
  - [Github structure](#github-structure)
- [Architecture](#architecture)
  - [Architecture Overview](#architecture-overview)
    - [Presentation Layer (UI)](#presentation-layer-ui)
    - [Business Logic Layer](#business-logic-layer)
    - [Data Layer](#data-layer)
    - [System Architecture Diagram](#system-architecture-diagram)
      - [Detailed Component Breakdown](#detailed-component-breakdown)
- [Authors and License](#authors-and-license)
  - [Code Contributors](#code-contributors)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->
  <hr>

# Description

**Desktop-Gallery** is a powerful image management tool built with **C++23** and **Qt6**. It is designed to handle large photo collections efficiently using a multithreaded architecture and a local SQLite database.

Beyond viewing images, it serves as a comprehensive **Metadata Editor** and **Workflow Tool**, allowing photographers to synchronize EXIF/IPTC/XMP data, geocode images via OpenStreetMap, visualize locations on an interactive map, and bulk-export assets to WebP or upload them directly to a CrowQtServer ([Crow Web-Gallery](https://github.com/Zheng-Bote/web-gallery_webserver)) instance.

## Key Features

✅ runs on Linux

✅ runs on MacOS

✅ runs on Windows

### 🚀 Performance & Core

- **Database-Driven:** High-performance SQLite backend with caching for instant search and filtering.
- **Multithreading:** Asynchronous recursive folder scanning and thumbnail generation.
- **Cross-Platform:** Runs natively on Linux, macOS, and Windows.

### 🌍 Geo-Intelligence

- **Reverse Geocoding:** Automatically fetch address data (City, Street, Country) based on GPS coordinates using **OpenStreetMap (Nominatim)**.
- **Interactive Map View:** Visualize selected images on a map using Leaflet.js integration.
- **GPS Data Management:** Read, write, and propagate GPS metadata.

### 📝 Metadata Management

- **Comprehensive Editor:** View and edit **EXIF**, **IPTC**, and **XMP** data.
- **Smart Synchronization:** Automatically syncs metadata between standards (e.g., changing EXIF Copyright updates IPTC and XMP fields instantly).
- **Bulk Operations:** Write default metadata (Copyright, Author) to multiple selected images at once.
- **Smart Rename:** Bulk rename files based on their creation timestamp.

### ☁️ Export & Cloud

- **WebP Bulk Export:** Convert images to WebP with customizable quality, resizing, and watermarking.
- **Cloud Upload Client:** Integrated client to upload images and metadata to a CrowQtServer (supports JWT Auth & Refresh Tokens).

## see also

| Product                                                                            | Description                                                              |
| ---------------------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| [CrowQtServer](https://github.com/Zheng-Bote/web-gallery_webserver)                | Crow ReST API Server with Qt6 & Worker (Inbox-Pattern) for Web-Gallery   |
| [Webserver Admin](https://github.com/Zheng-Bote/web-gallery_webserver-admin)       | extending CrowQtServer API Server to SCS (web-based user management etc) |
| [Web-Gallery frontend](https://github.com/Zheng-Bote/web-gallery_gallery-frontend) | Angular Web-Frontend for Web-Gallery                                     |
| [Desktop-Gallery](https://github.com/Zheng-Bote/web-gallery_desktop-gallery)       | Desktop-Gallery image management and uploader to Web-Gallery             |
| [CrowQtClient](https://github.com/Zheng-Bote/crow_webserver_desktop-client)        | simple Desktop Client to upload Pictures to the Web-Gallery              |
| [CliImgClient](https://github.com/Zheng-Bote/qt_files_photo-gallery)               | Commandline Batch Image processing and uploader to Web-Gallery           |

<p align="right">(<a href="#top">back to top</a>)</p>

# Getting Started

## Prerequisites

To build this project, you need the following dependencies installed:

- **CMake** (3.16+)
- **C++ Compiler** supporting C++23 (GCC 13+, Clang 16+, MSVC 2022)
- **Qt6** (Core, Gui, Widgets, Sql, Concurrent, Network, WebEngineWidgets, LinguistTools)
- **Exiv2** (Library for image metadata)

## Build Instructions

```bash
# 1. Clone the repository
git clone [https://github.com/Zheng-Bote/qt_desktop-gallery.git](https://github.com/Zheng-Bote/qt_desktop-gallery.git)
cd qt_desktop-gallery

# 2. Create build directory
mkdir build && cd build

# 3. Configure with CMake
cmake ..

# 4. Build
cmake --build .
```

### Usage Workflow

#### Import

Use File -> Open Folder (Recursive) to scan your image library.

#### Organize

Select images to view details. Use the context menu to copy GPS data or paths.

#### Edit Metadata

Double-click metadata fields to edit. Use Metadata -> Write default... for bulk updates.

#### Geocode

Select images with GPS data and run Metadata -> Address lookup to auto-fill location tags.

#### Visualize

Use Pictures -> Show selected on Map to see locations.

#### Export/Upload

Use the WebP Export menu for local conversion or Upload to Server to push to your web gallery.

<p align="right">(<a href="#top">back to top</a>)</p>

# Documentation & Screenshots

## Album View

Main interface showing the database-driven thumbnail grid.

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/start.png)

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/gps_copyright_view.png)

## Metadata Editor & Sync

Edit EXIF, IPTC, and XMP data with automatic synchronization.

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/metadata.png)

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/metadata_2.png)

Upload to Web-Gallery (CowQtServer ReST API-Server)

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/sync.png)

## Context Menu

several Picture actions via context menu.

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/context_menu.png)

## Map View

Interactive map showing the location of selected images (Leaflet/OSM).

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/map_view_1.png)

## WebP Export

Bulk conversion tool with resizing and watermarking options.

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/webp_export.png)

## Internationalization (i18n)

Live language switching (English/German).

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/i18n.png)

## Detail View

Responsive Picture view with extended editing and export options.

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/detail_view.png)

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/rotate.png)

![App Screenshot](https://github.com/Zheng-Bote/web-gallery_desktop-gallery/blob/main/docs/img/map_view_2.png)

<p align="right">(<a href="#top">back to top</a>)</p>

## Project Structure

```
.
├── CMakeLists.txt # Main build configuration
├── LICENSE # MIT License
├── README.md # Project documentation
├── configure
│   └── rz_config.hpp.in # Version and Project defines
├── i18n # Translation files (.ts and .qm)
│   ├── gallery_de.ts
│   └── gallery_en.ts
├── include # Header files (.hpp)
│   ├── DatabaseManager.hpp # SQLite encapsulation
│   ├── LoginDialog.hpp # Server Auth UI
│   ├── MapWindow.hpp # QWebEngine Map View
│   ├── MainWindow.hpp # Main GUI Logic
│   ├── UploadManager.hpp # Network/Upload Logic
│   ├── rz_metadata.hpp # Metadata Definitions & Mappings
│   └── rz_photo.hpp # Image & Exiv2 Wrapper
├── resources # Icons and Assets
│   └── img
├── src # Source files (.cpp)
│   ├── DatabaseManager.cpp
│   ├── ImageIndexer.cpp # Multithreaded Scanner
│   ├── MainWindow.cpp
│   ├── MapWindow.cpp
│   ├── ThumbnailDelegate.cpp # Custom Grid Rendering
│   ├── UploadManager.cpp
│   ├── main.cpp # Entry Point
│   ├── picture_widget.cpp # Detail View
│   └── rz_photo.cpp # Exiv2 Implementation
```

## Github structure

<!-- readme-tree end -->

<!-- readme-tree start -->

<p align="right">(<a href="#top">back to top</a>)</p>

# Architecture

Based on the codebase we have built, the architecture of Desktop-Gallery is a modular, event-driven desktop application leveraging Qt's Model-View-Controller (MVC) framework.

## Architecture Overview

The application is structured into three main layers:

### Presentation Layer (UI)

Handles user interaction, views (Gallery, Map, Details), and dialogs. It uses Qt Widgets.

### Business Logic Layer

Handles the "heavy lifting" like image processing, metadata editing, uploading, and geocoding. This layer is often multithreaded.

### Data Layer

Manages persistent storage (SQLite database), file system access, and configuration settings.

### System Architecture Diagram

This diagram visualizes how the different classes and modules interact with each other.

#### Detailed Component Breakdown

1. Main Window & Navigation (The Controller)
   MainWindow: The central hub. It orchestrates the entire application flow. It initializes the database, manages the layout (QSplitter), creates menus, and handles global events (like language switching).

LoginDialog: A modal dialog for authenticating with the CrowQtServer.

2. Data Models (The "M" in MVC)
   DatabaseManager: A Singleton class that wraps the SQLite database (gallery_cache.db). It handles connection opening/closing and schema creation.

QSqlTableModel: A Qt class that provides a direct link between the SQLite database and the UI views. It holds the image data (path, metadata, etc.).

QStandardItemModel: Manages the folder structure tree view on the left side.

3. Views & Delegates (The "V" in MVC)
   QListView (Gallery View): Displays the grid of images.

ThumbnailDelegate: A custom painter that renders the thumbnails, borders, and text overlays (Copyright/GPS) directly onto the QListView. It uses QPixmapCache for performance.

QTreeView: Displays the file system folder hierarchy.

MapWindow: Embeds a QWebEngineView to display OpenStreetMap/Leaflet.js for visualizing GPS coordinates.

4. Workers & Logic (Business Logic)
   ImageIndexer: A QRunnable (runs in a background thread via QThreadPool). It recursively scans folders for images, extracts metadata, and populates the database. This prevents the UI from freezing during large imports.

UploadManager: Handles the communication with the CrowQtServer. It manages the upload queue, performs authentication (JWT/Refresh tokens), and builds Multipart HTTP requests.

Photo (Wrapper): A utility class that wraps Exiv2. It provides a high-level API to read/write EXIF, IPTC, and XMP metadata, rotate images, and export to WebP.

rz_metadata: A static configuration namespace defining valid tags, mappings (Sync Rules), and default values.

5. External Services
   Nominatim (OSM): Accessed via QNetworkAccessManager for Reverse Geocoding (converting GPS coords to Address).

CrowQtServer: The remote backend (REST API) for uploading images.

Data Flow Example: Importing Images
User Action: User clicks "Open Folder (Recursive)".

Controller: MainWindow starts an ImageIndexer task in the global QThreadPool.

Worker: ImageIndexer iterates through files.

Helper: For each file, Photo reads metadata (EXIF/XMP).

Database: ImageIndexer inserts records into the SQLite DB via DatabaseManager.

View Update: The QSqlTableModel detects changes in the DB and signals the QListView.

Rendering: ThumbnailDelegate paints the new thumbnails in the grid.

**Technology Stack**

Language: C++23

Framework: Qt 6.9 (Core, Gui, Widgets, Sql, Network, Concurrent, WebEngineWidgets)

Metadata Library: Exiv2

Database: SQLite (via QtSql)

Build System: CMake

<p align="right">(<a href="#top">back to top</a>)</p>

# Authors and License

**License**
Distributed under the MIT License. See LICENSE for more information.

Copyright (c) 2025 ZHENG Robert

**Authors**

- [![Zheng Robert - Core Development](https://img.shields.io/badge/Github-Zheng_Robert-black?logo=github)](https://www.github.com/Zheng-Bote)

### Code Contributors

![Contributors](https://img.shields.io/github/contributors/Zheng-Bote/web-gallery_webserver-admin?color=dark-green)

[![Zheng Robert](https://img.shields.io/badge/Github-Zheng_Robert-black?logo=github)](https://www.github.com/Zheng-Bote)

<hr>

:vulcan_salute:
