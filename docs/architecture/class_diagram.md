<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Class Diagram](#class-diagram)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Class Diagram

```mermaid
classDiagram
    class MainWindow {
        +QString pathToImage
        +DatabaseManager* dbManager
        +ImageIndexer* indexer
        +UploadManager* uploadManager
        +QMap~QString, QString~ defaultMeta
        +MainWindow(QWidget *parent)
        +~MainWindow()
        -setupUi()
        -createMenu()
        -loadLanguage(QString locale)
    }

    class Photo {
        +QString fileAbsolutePath
        +QString fileName
        +Photo(const QString& imageInput)
        +bool writeAllMetadata(DefaultMetaStruct meta)
        +bool convertImages(int quality)
        +QPointF getGpsLatLon()
    }

    class DatabaseManager {
        +QSqlDatabase db
        +static DatabaseManager& instance()
        +bool init()
        +bool clearTable()
        +bool insertData(...)
    }

    class ImageIndexer {
        +QList~QString~ fileList
        +void run()
    }

    class UploadManager {
        +QString jwtToken
        +void setServerUrl(QString url)
        +void login(QString user, QString pass)
        +void uploadFiles(...)
        -void startNextUpload()
    }

    class MapWindow {
        +QWebEngineView* webView
        +void showImages(QList~QString~ paths)
        -QString generateHtml(...)
    }

    class PictureWidget {
        +QPixmap* picture
        +void setImage(QString path)
        -void readSrcExif()
        -void readSrcIptc()
        -void readSrcXmp()
    }

    MainWindow "1" *-- "1" DatabaseManager
    MainWindow "1" *-- "1" ImageIndexer
    MainWindow "1" *-- "1" UploadManager
    MainWindow "1" *-- "N" PictureWidget
    MainWindow "1" *-- "1" MapWindow
    PictureWidget "1" ..> "1" Photo : "Uses for Meta/Exiv2"
    ImageIndexer "1" ..> "1" Photo : "Uses to Extract Meta"
    UploadManager "1" ..> "1" Photo : "Extracts Copyright/GPS for API"
```
