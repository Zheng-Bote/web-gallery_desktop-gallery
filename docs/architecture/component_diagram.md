<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Component Diagram](#component-diagram)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Component Diagram

```mermaid
flowchart TD
    subgraph App [Desktop-Gallery]
        MainUI[MainUI]
        MapModule["MapModule (QtWebEngine)"]
        DatabaseModule["DatabaseModule (QtSql)"]
        UploadClient["UploadClient (QtNetwork)"]
        ImageProcessor["ImageProcessor (Exiv2 wrapper)"]
    end

    MainUI -->|Read/Write Index| DatabaseModule
    MainUI -->|Initiate Transfer| UploadClient
    MainUI -->|Pass GPS Data| MapModule
    DatabaseModule -->|Extract Metadata| ImageProcessor
    ImageProcessor -->|Provide Thumbnails & Meta| MainUI

    subgraph DB [SQLite 3]
        gallery_db[(gallery.db)]
    end
    
    subgraph Cloud [External Services]
        Nominatim["Nominatim (OSM)"]
        CrowQtServer["CrowQtServer"]
    end

    DatabaseModule -.-> gallery_db
    MainUI -.->|Reverse Geocode Request| Nominatim
    UploadClient -.->|REST API JWT/Multipart| CrowQtServer
```
