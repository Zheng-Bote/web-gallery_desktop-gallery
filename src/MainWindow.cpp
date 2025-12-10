/**
 * @file MainWindow.cpp
 * @author ZHENG Robert
 * @brief Main Application Logic
 */
#include "MainWindow.hpp"
#include "DatabaseManager.hpp"
#include "ImageIndexer.hpp"
#include "ThumbnailDelegate.hpp"
#include "picture_widget.h"
#include "DefaultMetaWidget.hpp" 
#include "rz_photo.hpp" 

#include <QApplication>
#include <QHeaderView>
#include <QThreadPool>
#include <QStatusBar>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QMenuBar>
#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>
#include <QtConcurrent>
#include <QPixmapCache>
#include <QSettings>
#include <QInputDialog>

// Network & JSON
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

// --- Helper für Sync der Default-Struktur ---
static void synchronizeMetaStruct(RzMetadata::DefaultMetaStruct& meta) {
    // EXIF -> XMP & IPTC
    QHashIterator<QString, QString> iE(meta.exifDefault);
    while (iE.hasNext()) {
        iE.next();
        if (!iE.value().isEmpty()) {
            if (RzMetadata::exif_to_xmp.contains(iE.key())) 
                meta.xmpDefault[RzMetadata::exif_to_xmp[iE.key()]] = iE.value();
            if (RzMetadata::exif_to_iptc.contains(iE.key())) 
                meta.iptcDefault[RzMetadata::exif_to_iptc[iE.key()]] = iE.value();
        }
    }
    // IPTC -> EXIF & XMP
    QHashIterator<QString, QString> iI(meta.iptcDefault);
    while (iI.hasNext()) {
        iI.next();
        if (!iI.value().isEmpty()) {
            if (RzMetadata::iptc_to_exif.contains(iI.key())) 
                meta.exifDefault[RzMetadata::iptc_to_exif[iI.key()]] = iI.value();
            if (RzMetadata::iptc_to_xmp.contains(iI.key())) 
                meta.xmpDefault[RzMetadata::iptc_to_xmp[iI.key()]] = iI.value();
        }
    }
    // XMP -> EXIF & IPTC
    QHashIterator<QString, QString> iX(meta.xmpDefault);
    while (iX.hasNext()) {
        iX.next();
        if (!iX.value().isEmpty()) {
            if (RzMetadata::xmp_to_exif.contains(iX.key())) 
                meta.exifDefault[RzMetadata::xmp_to_exif[iX.key()]] = iX.value();
            if (RzMetadata::xmp_to_iptc.contains(iX.key())) 
                meta.iptcDefault[RzMetadata::xmp_to_iptc[iX.key()]] = iX.value();
        }
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel);
    
    // Settings laden (Portable: im App-Ordner)
    QString iniPath = QApplication::applicationDirPath() + "/settings.ini";
    m_settings = new QSettings(iniPath, QSettings::IniFormat, this);

    // Network Setup
    m_netManager = new QNetworkAccessManager(this);
    connect(m_netManager, &QNetworkAccessManager::finished, this, &MainWindow::onGeoNamesReply);

    setupDatabase();
    setupUi();
    createMenu(); 
    
    resize(1200, 800);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupDatabase() {
    DatabaseManager::instance().init();
}

void MainWindow::setupUi() {
    m_splitter = new QSplitter(this);
    setCentralWidget(m_splitter);

    m_treeView = new QTreeView(this);
    m_folderModel = new QStandardItemModel(this);
    m_folderModel->setHorizontalHeaderLabels({tr("Folders")});
    
    m_treeView->setModel(m_folderModel);
    m_treeView->setHeaderHidden(true);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_splitter->addWidget(m_treeView);

    m_galleryView = new QListView(this);
    m_galleryView->setViewMode(QListView::IconMode);
    m_galleryView->setResizeMode(QListView::Adjust);
    m_galleryView->setSpacing(10);
    m_galleryView->setUniformItemSizes(true);
    m_galleryView->setGridSize(QSize(280, 280)); 
    m_galleryView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_galleryModel = new QSqlTableModel(this, DatabaseManager::instance().getDb());
    m_galleryModel->setTable("images");
    m_galleryModel->select(); 
    
    m_galleryView->setModel(m_galleryModel);
    m_galleryView->setModelColumn(2); 

    m_galleryView->setItemDelegate(new ThumbnailDelegate(this));

    m_splitter->addWidget(m_galleryView);
    m_splitter->setSizes(QList<int>{300, 900});

    connect(m_treeView, &QTreeView::clicked, this, &MainWindow::onFolderClicked);
    connect(m_galleryView, &QListView::doubleClicked, this, &MainWindow::onGalleryDoubleClicked);
    
    m_galleryView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_galleryView, &QListView::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

void MainWindow::createMenu() {
    menuBar()->setNativeMenuBar(false); 

    // 1. FILE MENU
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* openAct = fileMenu->addAction(tr("Open Folder..."));
    connect(openAct, &QAction::triggered, this, &MainWindow::openSrcFolder);
    QAction* openRecAct = fileMenu->addAction(tr("Open Folder (Recursive)..."));
    connect(openRecAct, &QAction::triggered, this, &MainWindow::openSrcFolderRekursive);
    fileMenu->addSeparator();
    QAction* exitAct = fileMenu->addAction(tr("Exit"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // 2. SETTINGS MENU
    QMenu* settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QAction* geoUserAct = settingsMenu->addAction(tr("Configure GeoNames User..."));
    connect(geoUserAct, &QAction::triggered, this, &MainWindow::openSettingsDialog);

    // 3. METADATA MENU
    QMenu* metaMenu = menuBar()->addMenu(tr("&Metadata"));
    QAction* metaAct = metaMenu->addAction(tr("Edit Default Metadata..."));
    connect(metaAct, &QAction::triggered, this, &MainWindow::showDefaultMetaWidget);
    
    metaMenu->addSeparator();
    QAction* writeCpAct = metaMenu->addAction(tr("Write default Copyright to selected"));
    connect(writeCpAct, &QAction::triggered, this, &MainWindow::writeDefaultCopyrightToSelection);
    QAction* writeGpsAct = metaMenu->addAction(tr("Write default GPS to selected"));
    connect(writeGpsAct, &QAction::triggered, this, &MainWindow::writeDefaultGpsToSelection);
    QAction* writeAllAct = metaMenu->addAction(tr("Write ALL default Metadata to selected"));
    connect(writeAllAct, &QAction::triggered, this, &MainWindow::writeAllDefaultsToSelection);
    
    metaMenu->addSeparator();
    QAction* geoLookupAct = metaMenu->addAction(tr("Address lookup for selected pictures (GeoNames)"));
    connect(geoLookupAct, &QAction::triggered, this, &MainWindow::startGeoNamesLookup);

    // 4. PICTURES MENU
    QMenu* picMenu = menuBar()->addMenu(tr("&Pictures"));

    // View Options
    QMenu* viewMenu = picMenu->addMenu(tr("View Options"));
    actShowCopyright = viewMenu->addAction(tr("Display Copyright Owner"));
    actShowCopyright->setCheckable(true);
    actShowGps = viewMenu->addAction(tr("Display GPS Data"));
    actShowGps->setCheckable(true);

    auto updateDelegate = [this]() {
        ThumbnailDelegate* del = qobject_cast<ThumbnailDelegate*>(m_galleryView->itemDelegate());
        if (del) {
            del->setShowCopyright(actShowCopyright->isChecked());
            del->setShowGps(actShowGps->isChecked());
            m_galleryView->viewport()->update();
        }
    };
    connect(actShowCopyright, &QAction::toggled, this, updateDelegate);
    connect(actShowGps, &QAction::toggled, this, updateDelegate);

    picMenu->addSeparator();

    // WebP Options
    QMenu* webpMenu = picMenu->addMenu(tr("WebP Export"));
    actWebpOversize = webpMenu->addAction(tr("Increase too small pictures"));
    actWebpOversize->setCheckable(true); actWebpOversize->setChecked(true);
    actWebpOverwrite = webpMenu->addAction(tr("Overwrite existing WebP"));
    actWebpOverwrite->setCheckable(true);
    actWebpWatermark = webpMenu->addAction(tr("Watermark WebP"));
    actWebpWatermark->setCheckable(true); actWebpWatermark->setChecked(true);
    webpMenu->addSeparator();
    actWebpRename = webpMenu->addAction(tr("Rename Export to Timestamp"));
    actWebpRename->setCheckable(true);
    webpMenu->addSeparator();
    QAction* actExportWebp = webpMenu->addAction(tr("Export selected to all WebP sizes"));
    connect(actExportWebp, &QAction::triggered, this, &MainWindow::exportSelectedToWebP);

    picMenu->addSeparator();

    QAction* actSelectAll = picMenu->addAction(tr("Select all Pictures"));
    actSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(actSelectAll, &QAction::triggered, this, &MainWindow::selectAllImages);

    QAction* actRename = picMenu->addAction(tr("Rename selected to Timestamp"));
    connect(actRename, &QAction::triggered, this, &MainWindow::renameSelectedImages);
}

// --- FOLDER LOGIC ---

void MainWindow::openSrcFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder"), QDir::homePath());
    if (dir.isEmpty()) return;
    addFolderToTree(dir, false);
    m_statusLabel->setText(tr("Scanning: ") + dir);
    ImageIndexer* indexer = new ImageIndexer(dir); 
    indexer->setAutoDelete(true);
    connect(indexer, &ImageIndexer::finished, this, &MainWindow::onIndexerFinished);
    QThreadPool::globalInstance()->start(indexer);
}

void MainWindow::openSrcFolderRekursive() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder (Recursive)"), QDir::homePath());
    if (dir.isEmpty()) return;
    addFolderToTree(dir, true);
    m_statusLabel->setText(tr("Scanning recursively: ") + dir);
    ImageIndexer* indexer = new ImageIndexer(dir);
    indexer->setAutoDelete(true);
    connect(indexer, &ImageIndexer::finished, this, &MainWindow::onIndexerFinished);
    QThreadPool::globalInstance()->start(indexer);
}

void MainWindow::addFolderToTree(const QString& path, bool recursive) {
    QDir dir(path);
    if (!dir.exists()) return;
    QFileIconProvider iconProvider;
    QStandardItem* rootItem = new QStandardItem(iconProvider.icon(QFileIconProvider::Folder), dir.dirName());
    rootItem->setData(dir.absolutePath(), Qt::UserRole); 
    m_folderModel->appendRow(rootItem);
    if (recursive) addSubfoldersRecursively(path, rootItem);
    m_treeView->expand(rootItem->index());
}

void MainWindow::addSubfoldersRecursively(const QString& path, QStandardItem* parentItem) {
    QDir dir(path);
    QFileIconProvider iconProvider;
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QFileInfo& entry : dir.entryInfoList()) {
        QStandardItem* subItem = new QStandardItem(iconProvider.icon(QFileIconProvider::Folder), entry.fileName());
        subItem->setData(entry.absoluteFilePath(), Qt::UserRole);
        parentItem->appendRow(subItem);
        addSubfoldersRecursively(entry.absoluteFilePath(), subItem);
    }
}

// --- EVENTS ---

void MainWindow::onFolderClicked(const QModelIndex& index) {
    QString rawPath = index.data(Qt::UserRole).toString();
    if (rawPath.isEmpty()) return;
    QString path = QDir::cleanPath(rawPath);
    m_statusLabel->setText(tr("Selected: ") + path);
    QString dbFilter = QString("folder_path = '%1'").arg(path.replace("'", "''"));
    m_galleryModel->setFilter(dbFilter);
    m_galleryModel->select();
}

void MainWindow::onGalleryDoubleClicked(const QModelIndex &index) {
    QModelIndex pathIndex = m_galleryModel->index(index.row(), 1);
    QString filePath = pathIndex.data().toString();
    if (filePath.isEmpty()) return;
    PictureWidget *viewer = new PictureWidget();
    viewer->setImage(filePath);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    connect(viewer, &PictureWidget::imageChanged, this, &MainWindow::onImageChanged);
    viewer->show();
}

void MainWindow::onIndexerFinished() {
    m_statusLabel->setText(tr("Scan complete."));
    m_galleryModel->select(); 
}

void MainWindow::onImageChanged(const QString& path) {
    QPixmapCache::remove("thumb_" + path);
    m_galleryView->viewport()->update();
}

// --- ACTIONS: GENERAL ---

void MainWindow::selectAllImages() {
    m_galleryView->selectAll();
    QSet<int> rows;
    for (const QModelIndex& idx : m_galleryView->selectionModel()->selectedIndexes()) {
        rows.insert(idx.row());
    }
    m_statusLabel->setText(tr("Selected %1 images.").arg(rows.size()));
}

void MainWindow::renameSelectedImages() {
    QList<QString> files = getSelectedFilePaths();
    if (files.isEmpty()) { QMessageBox::information(this, tr("Rename"), tr("No images selected.")); return; }
    int count = 0; int failed = 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QSqlQuery query(DatabaseManager::instance().getDb());
    for (const QString& oldPath : files) {
        Photo p(oldPath);
        auto [success, newPath] = p.renameImageToTimestamp();
        if (success) {
            query.prepare("UPDATE images SET path = ?, filename = ? WHERE path = ?");
            query.addBindValue(newPath); query.addBindValue(QFileInfo(newPath).fileName()); query.addBindValue(oldPath);
            if(query.exec()) { count++; QPixmapCache::remove("thumb_" + oldPath); } else failed++;
        } else failed++;
    }
    QApplication::restoreOverrideCursor();
    if (count > 0) { m_statusLabel->setText(tr("Renamed %1 images.").arg(count)); m_galleryModel->select(); }
    if (failed > 0) QMessageBox::warning(this, tr("Rename Results"), tr("Renamed: %1\nFailed: %2").arg(count).arg(failed));
}

// --- ACTIONS: METADATA WRITE ---

void MainWindow::writeDefaultCopyrightToSelection() {
    RzMetadata::DefaultMetaStruct cpMeta;
    auto filter = [](const QHash<QString, QString>& source, QHash<QString, QString>& target) {
        QHashIterator<QString, QString> i(source);
        while (i.hasNext()) { i.next();
            QString k = i.key().toLower();
            if (k.contains("copyright") || k.contains("rights") || k.contains("artist") || k.contains("creator") || k.contains("byline"))
                target.insert(i.key(), i.value());
        }
    };
    filter(defaultMeta.exifDefault, cpMeta.exifDefault);
    filter(defaultMeta.iptcDefault, cpMeta.iptcDefault);
    filter(defaultMeta.xmpDefault, cpMeta.xmpDefault);

    if (cpMeta.exifDefault.isEmpty() && cpMeta.iptcDefault.isEmpty() && cpMeta.xmpDefault.isEmpty()) {
        QMessageBox::information(this, tr("Metadata"), tr("No Default Copyright data set.")); return;
    }
    QList<QString> files = getSelectedFilePaths();
    int count = 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (const QString& path : files) {
        Photo p(path);
        if (p.writeAllMetadata(cpMeta)) { updateDbMetadata(path); count++; }
    }
    QApplication::restoreOverrideCursor();
    m_galleryView->viewport()->update();
    m_statusLabel->setText(tr("Updated Copyright in %1 images.").arg(count));
}

void MainWindow::writeDefaultGpsToSelection() {
    RzMetadata::DefaultMetaStruct gpsMeta;
    QHashIterator<QString, QString> i(defaultMeta.exifDefault);
    while (i.hasNext()) { i.next();
        if (i.key().startsWith("Exif.GPSInfo")) gpsMeta.exifDefault.insert(i.key(), i.value());
    }
    if (gpsMeta.exifDefault.isEmpty()) { QMessageBox::information(this, tr("Metadata"), tr("No Default GPS data set.")); return; }
    QList<QString> files = getSelectedFilePaths();
    int count = 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (const QString& path : files) {
        Photo p(path);
        if (p.writeAllMetadata(gpsMeta)) { updateDbMetadata(path); count++; }
    }
    QApplication::restoreOverrideCursor();
    m_galleryView->viewport()->update();
    m_statusLabel->setText(tr("Updated GPS in %1 images.").arg(count));
}

void MainWindow::writeAllDefaultsToSelection() {
    QList<QString> files = getSelectedFilePaths();
    int count = 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (const QString& path : files) {
        Photo p(path);
        if (p.writeAllMetadata(defaultMeta)) { updateDbMetadata(path); count++; }
    }
    QApplication::restoreOverrideCursor();
    m_galleryView->viewport()->update();
    m_statusLabel->setText(tr("Updated Metadata in %1 images.").arg(count));
}

// --- ACTIONS: METADATA SET DEFAULT ---

void MainWindow::setCopyrightAsDefaultFromCurrent() {
    QModelIndex idx = m_galleryView->currentIndex();
    if (!idx.isValid()) return;
    Photo p(m_galleryModel->index(idx.row(), 1).data().toString());
    int found = 0;
    auto readAndSet = [&](const QMap<QString, QString>& keysToCheck, QHash<QString, QString>& target) {
        QMapIterator<QString, QString> i(keysToCheck);
        while (i.hasNext()) { i.next();
            QString k = i.key(); QString kLow = k.toLower();
            if (kLow.contains("copyright") || kLow.contains("rights") || kLow.contains("artist") || kLow.contains("creator") || kLow.contains("byline")) {
                QString val = p.getRawTagValue(k);
                if (!val.isEmpty()) { target[k] = val; found++; }
            }
        }
    };
    readAndSet(RzMetadata::exifDefaultTagsList, defaultMeta.exifDefault);
    readAndSet(RzMetadata::iptcDefaultTagsList, defaultMeta.iptcDefault);
    readAndSet(RzMetadata::xmpDefaultTagsList, defaultMeta.xmpDefault);
    if (found > 0) {
        synchronizeMetaStruct(defaultMeta);
        QMessageBox::information(this, tr("Defaults Set"), tr("Loaded %1 copyright tags (synced).").arg(found));
        showDefaultMetaWidget();
    } else QMessageBox::warning(this, tr("Defaults"), tr("No copyright information found."));
}

void MainWindow::setGpsAsDefaultFromCurrent() {
    QModelIndex idx = m_galleryView->currentIndex();
    if (!idx.isValid()) return;
    Photo p(m_galleryModel->index(idx.row(), 1).data().toString());
    QStringList gpsTags = {"Exif.GPSInfo.GPSLatitude", "Exif.GPSInfo.GPSLatitudeRef", "Exif.GPSInfo.GPSLongitude", "Exif.GPSInfo.GPSLongitudeRef", "Exif.GPSInfo.GPSAltitude", "Exif.GPSInfo.GPSAltitudeRef", "Exif.GPSInfo.GPSMapDatum"};
    int found = 0;
    for (const QString& tag : gpsTags) {
        QString val = p.getRawTagValue(tag);
        if (!val.isEmpty()) { defaultMeta.exifDefault[tag] = val; found++; }
    }
    if (found > 0) {
        QMessageBox::information(this, tr("Defaults Set"), tr("Loaded GPS coordinates."));
        showDefaultMetaWidget();
    } else QMessageBox::warning(this, tr("Defaults"), tr("No GPS data found."));
}

void MainWindow::setAllAsDefaultFromCurrent() {
    QModelIndex idx = m_galleryView->currentIndex();
    if (!idx.isValid()) return;
    Photo p(m_galleryModel->index(idx.row(), 1).data().toString());
    auto readAll = [&](const QMap<QString, QString>& keysList, QHash<QString, QString>& target) {
        QMapIterator<QString, QString> i(keysList);
        while (i.hasNext()) { i.next();
            QString val = p.getRawTagValue(i.key());
            if (!val.isEmpty()) target[i.key()] = val;
        }
    };
    readAll(RzMetadata::exifDefaultTagsList, defaultMeta.exifDefault);
    readAll(RzMetadata::iptcDefaultTagsList, defaultMeta.iptcDefault);
    readAll(RzMetadata::xmpDefaultTagsList, defaultMeta.xmpDefault);
    setGpsAsDefaultFromCurrent();
    synchronizeMetaStruct(defaultMeta);
    QMessageBox::information(this, tr("Defaults Set"), tr("Loaded metadata tags."));
    showDefaultMetaWidget();
}

void MainWindow::updateDbMetadata(const QString& path) {
    Photo p(path);
    QString cp = p.getXmpCopyrightOwner();
    QString gps = p.getGpsDecimalString();
    QSqlQuery query(DatabaseManager::instance().getDb());
    query.prepare("UPDATE images SET copyright = ?, gps_decimal = ? WHERE path = ?");
    query.addBindValue(cp); query.addBindValue(gps); query.addBindValue(path);
    query.exec();
}

// --- REVERSE GEOCODING (OSM NOMINATIM) ---

void MainWindow::openSettingsDialog() {
    // Nominatim benötigt keinen User, aber falls du später Settings brauchst:
    QMessageBox::information(this, tr("Settings"), 
        tr("OpenStreetMap Nominatim is used.\nNo configuration required."));
}

void MainWindow::startGeoNamesLookup() {
    // KORREKTUR: Kein Username-Check mehr nötig für Nominatim
    
    QList<QString> files = getSelectedFilePaths();
    if (files.isEmpty()) return;

    m_geoLookupQueue.clear();
    for(const QString& f : files) m_geoLookupQueue.enqueue(f);
    
    m_geoTotalCount = files.size();
    m_statusLabel->setText(tr("Starting OSM Nominatim lookup for %1 images...").arg(m_geoTotalCount));
    
    processNextGeoLookup();
}

void MainWindow::processNextGeoLookup() {
    if (m_geoLookupQueue.isEmpty()) {
        m_statusLabel->setText(tr("Lookup finished."));
        QMessageBox::information(this, tr("Finished"), tr("Address lookup completed."));
        m_galleryModel->select(); 
        return;
    }

    QString path = m_geoLookupQueue.head();
    Photo p(path);
    QPointF gps = p.getGpsLatLon();

    // Check auf gültige Koordinaten (0.0 ist ungültig)
    if (qAbs(gps.x()) < 0.0001 && qAbs(gps.y()) < 0.0001) {
        qWarning() << "Skipping file without GPS:" << path;
        m_geoLookupQueue.dequeue();
        processNextGeoLookup();
        return;
    }

    // --- NOMINATIM API ---
    QUrl url("https://nominatim.openstreetmap.org/reverse");
    QUrlQuery query;
    query.addQueryItem("format", "json");
    query.addQueryItem("lat", QString::number(gps.x(), 'f', 6)); // Latitude
    query.addQueryItem("lon", QString::number(gps.y(), 'f', 6)); // Longitude (heißt hier 'lon', nicht 'lng')
    query.addQueryItem("zoom", "18"); // Maximale Genauigkeit
    query.addQueryItem("addressdetails", "1");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, path);
    
    // WICHTIG: User-Agent setzen (Pflicht bei OSM!), sonst HTTP 403
    request.setRawHeader("User-Agent", "DesktopGalleryApp/1.0");
    
    // WICHTIG: Sprache auf Englisch erzwingen
    request.setRawHeader("Accept-Language", "en");

    m_statusLabel->setText(tr("Lookup: %1 / %2 ...").arg(m_geoTotalCount - m_geoLookupQueue.size() + 1).arg(m_geoTotalCount));
    
    m_netManager->get(request);
}

void MainWindow::onGeoNamesReply(QNetworkReply* reply) {
    // 1. Queue Management
    if (!m_geoLookupQueue.isEmpty()) m_geoLookupQueue.dequeue();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error:" << reply->errorString();
        // Kurze Pause bei Fehler, um Server nicht zu hämmern?
        // Einfachheitshalber direkt weiter:
        processNextGeoLookup(); 
        return;
    }

    QString path = reply->request().attribute(QNetworkRequest::User).toString();
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    // Nominatim Fehlererkennung
    if (obj.contains("error")) {
        qWarning() << "Nominatim API Error:" << obj["error"].toString();
        processNextGeoLookup(); 
        return;
    }

    // Daten extrahieren
    QJsonObject addr = obj["address"].toObject();

    if (!addr.isEmpty()) {
        RzMetadata::DefaultMetaStruct meta;
        
        // OSM Felder Mapping (etwas anders als GeoNames)
        // Doku: https://nominatim.org/release-docs/latest/api/Output/
        
        // 1. Stadt ermitteln (Hierarchie beachten)
        QString city = addr["city"].toString();
        if (city.isEmpty()) city = addr["town"].toString();
        if (city.isEmpty()) city = addr["village"].toString();
        if (city.isEmpty()) city = addr["municipality"].toString();
        if (city.isEmpty()) city = addr["hamlet"].toString();

        // 2. Straße
        QString road = addr["road"].toString();
        QString house_number = addr["house_number"].toString();
        QString fullStreet = road;
        if (!house_number.isEmpty()) fullStreet += " " + house_number;

        // 3. Andere Felder
        QString state = addr["state"].toString();
        if (state.isEmpty()) state = addr["county"].toString(); // Fallback
        
        QString postcode = addr["postcode"].toString();
        QString country = addr["country"].toString(); // Name (z.B. "Germany")
        QString countryCode = addr["country_code"].toString().toUpper(); // Code (z.B. "de" -> "DE")

        // --- Schreiben in Metadaten ---

        // Stadt
        if (!city.isEmpty()) {
            meta.iptcDefault["Iptc.Application2.City"] = city;
            meta.xmpDefault["Xmp.photoshop.City"] = city;
        }

        // Straße
        if (!fullStreet.isEmpty()) {
            meta.iptcDefault["Iptc.Application2.SubLocation"] = fullStreet;
            meta.xmpDefault["Xmp.iptc.Location"] = fullStreet;
            meta.xmpDefault["Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr"] = fullStreet;
        }

        // Staat / Bundesland
        if (!state.isEmpty()) {
            meta.iptcDefault["Iptc.Application2.ProvinceState"] = state;
            meta.xmpDefault["Xmp.photoshop.State"] = state;
        }

        // Land
        if (!countryCode.isEmpty()) {
            meta.iptcDefault["Iptc.Application2.CountryCode"] = countryCode;
            meta.xmpDefault["Xmp.iptc.CountryCode"] = countryCode;
        }
        if (!country.isEmpty()) {
            meta.iptcDefault["Iptc.Application2.CountryName"] = country;
            meta.xmpDefault["Xmp.photoshop.Country"] = country;
        }

        // PLZ
        if (!postcode.isEmpty()) {
            meta.xmpDefault["Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode"] = postcode;
        }

        // Speichern
        Photo p(path);
        if (p.writeAllMetadata(meta)) {
            qDebug() << "Nominatim Data written to:" << QFileInfo(path).fileName() 
                     << "[" << city << "," << countryCode << "]";
        }
    } else {
        qWarning() << "No address found for:" << path;
    }

    // Nächstes Bild
    processNextGeoLookup();
}

void MainWindow::showContextMenu(const QPoint& pos) {
    QModelIndex index = m_galleryView->indexAt(pos);
    if (!index.isValid()) return;
    QMenu contextMenu(this);
    QAction* openAct = contextMenu.addAction(tr("Open"));
    connect(openAct, &QAction::triggered, this, [this, index](){ onGalleryDoubleClicked(index); });
    contextMenu.addSeparator();
    if (m_galleryView->selectionModel()->hasSelection()) {
        QAction* webpAct = contextMenu.addAction(tr("Export to WebP..."));
        connect(webpAct, &QAction::triggered, this, &MainWindow::exportSelectedToWebP);
        QAction* renameAct = contextMenu.addAction(tr("Rename to Timestamp"));
        connect(renameAct, &QAction::triggered, this, &MainWindow::renameSelectedImages);
        contextMenu.addSeparator();
        QAction* setDefCp = contextMenu.addAction(tr("Set Copyright as Default"));
        connect(setDefCp, &QAction::triggered, this, &MainWindow::setCopyrightAsDefaultFromCurrent);
        QAction* setDefGps = contextMenu.addAction(tr("Set GPS Data as Default"));
        connect(setDefGps, &QAction::triggered, this, &MainWindow::setGpsAsDefaultFromCurrent);
        QAction* setDefAll = contextMenu.addAction(tr("Set ALL Metadata as Default"));
        connect(setDefAll, &QAction::triggered, this, &MainWindow::setAllAsDefaultFromCurrent);
        contextMenu.addSeparator();
        QAction* gpsAct = contextMenu.addAction(tr("Copy GPS to Clipboard"));
        connect(gpsAct, &QAction::triggered, this, [this, index](){
            Photo p(m_galleryModel->index(index.row(), 1).data().toString());
            QString gps = p.getGpsString();
            if (!gps.isEmpty()) { QGuiApplication::clipboard()->setText(gps); m_statusLabel->setText(tr("GPS copied: ") + gps); } 
            else m_statusLabel->setText(tr("No GPS data found."));
        });
        QAction* pathAct = contextMenu.addAction(tr("Copy Path to Clipboard"));
        connect(pathAct, &QAction::triggered, this, [this, index](){
            QString path = m_galleryModel->index(index.row(), 1).data().toString();
            QGuiApplication::clipboard()->setText(path);
            m_statusLabel->setText(tr("Path copied."));
        });
    }
    contextMenu.exec(m_galleryView->viewport()->mapToGlobal(pos));
}

void MainWindow::exportSelectedToWebP() {
    QList<QString> files = getSelectedFilePaths();
    if (files.isEmpty()) { QMessageBox::information(this, tr("WebP Export"), tr("No images selected.")); return; }
    
    // Capture state variables for thread
    bool optOversize = actWebpOversize->isChecked();
    bool optOverwrite = actWebpOverwrite->isChecked();
    bool optWatermark = actWebpWatermark->isChecked();
    bool optRename = actWebpRename->isChecked();

    m_statusLabel->setText(tr("Exporting %1 images to WebP...").arg(files.size()));
    QFuture<void> future = QtConcurrent::run([files, this, optOversize, optOverwrite, optWatermark, optRename]() {
        int successCount = 0;
        for (const QString& path : files) {
            Photo photo(path);
            photo.setOversizeSmallerPicture(optOversize);
            photo.setOverwriteExistingWebp(optOverwrite);
            photo.setWatermarkWebp(optWatermark);
            photo.setRenameToTimestamp(optRename);
            if (photo.convertImages(75)) successCount++;
        }
        QMetaObject::invokeMethod(this, [this, successCount, total = files.size()](){
            m_statusLabel->setText(tr("Export finished: %1 / %2 successful.").arg(successCount).arg(total));
            QMessageBox::information(this, tr("WebP Export"), tr("Successfully exported %1 of %2 images.").arg(successCount).arg(total));
        });
    });
}

QList<QString> MainWindow::getSelectedFilePaths() {
    QList<QString> paths;
    QModelIndexList selected = m_galleryView->selectionModel()->selectedIndexes();
    QSet<int> processedRows;
    for (const QModelIndex& index : selected) {
        if (!processedRows.contains(index.row())) {
            processedRows.insert(index.row());
            QString path = m_galleryModel->index(index.row(), 1).data().toString();
            if (!path.isEmpty()) paths.append(path);
        }
    }
    return paths;
}

void MainWindow::showDefaultMetaWidget() {
    DefaultMetaWidget *widget = new DefaultMetaWidget();
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setDefaultMeta(&defaultMeta); 
    widget->show();
}

void MainWindow::loadLanguage(const QString& rLanguage) {
    if (m_currLang != rLanguage) {
        m_currLang = rLanguage;
        QLocale locale = QLocale(m_currLang);
        QLocale::setDefault(locale);
        qApp->removeTranslator(&m_translator);
        qApp->removeTranslator(&m_translatorQt);
        QString langPath = QApplication::applicationDirPath() + "/i18n";
        if (!QFile::exists(langPath)) langPath = ":/i18n"; 
        if (m_translator.load(QString("gallery_%1").arg(rLanguage), langPath)) qApp->installTranslator(&m_translator);
        if (m_translatorQt.load(QString("qt_%1").arg(rLanguage), langPath)) qApp->installTranslator(&m_translatorQt);
        m_statusLabel->setText(tr("Language changed to: ") + rLanguage.toUpper());
    }
}