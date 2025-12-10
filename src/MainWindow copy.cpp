#include "MainWindow.hpp"
#include "DatabaseManager.hpp"
#include "ImageIndexer.hpp"
#include "ThumbnailDelegate.hpp"
#include <QHeaderView>
#include <QThreadPool>
#include <QStatusBar>
#include <QDebug>     // Wichtig
#include <QDir>       // Wichtig
#include <QSqlError>  // Wichtig

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Statusbar initialisieren
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel);
    
    setupDatabase();
    setupUi();
    
    resize(1200, 800);
}

MainWindow::~MainWindow() {}

void MainWindow::setupDatabase() {
    DatabaseManager::instance().init();
}

void MainWindow::setupUi() {
    m_splitter = new QSplitter(this);
    setCentralWidget(m_splitter);

    // --- 1. Linker Bereich: Baumstruktur (Filesystem) ---
    m_treeView = new QTreeView(this);
    m_fileSystemModel = new QFileSystemModel(this);
    
    // Root Path setzen (Startet beim Home Verzeichnis oder Root)
    m_fileSystemModel->setRootPath(QDir::homePath());
    m_fileSystemModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    
    m_treeView->setModel(m_fileSystemModel);
    m_treeView->setRootIndex(m_fileSystemModel->index(QDir::homePath()));
    
    // Nur die "Name" Spalte anzeigen, Größe/Datum ausblenden für Clean-Look
    m_treeView->hideColumn(1);
    m_treeView->hideColumn(2);
    m_treeView->hideColumn(3);
    m_treeView->setHeaderHidden(true);

    m_splitter->addWidget(m_treeView);

    // --- 2. Rechter Bereich: Galerie (Datenbank View) ---
    m_galleryView = new QListView(this);
    m_galleryView->setViewMode(QListView::IconMode);
    m_galleryView->setResizeMode(QListView::Adjust);
    m_galleryView->setSpacing(10);
    m_galleryView->setUniformItemSizes(true); // Wichtig für Performance
    m_galleryView->setGridSize(QSize(280, 280));

    // SQL Model
    m_galleryModel = new QSqlTableModel(this, DatabaseManager::instance().getDb());
    m_galleryModel->setTable("images");
    
    // Wir zeigen Spalte "filename" (Index 2 in unserer DB-Erstellung) als DisplayRole
    // Das ThumbnailDelegate kümmert sich um das Bild (nutzt Spalte 'path')
    m_galleryModel->select(); 
    
    m_galleryView->setModel(m_galleryModel);
    m_galleryView->setModelColumn(2); // Filename anzeigen

    // Delegate für Bilder
    m_galleryView->setItemDelegate(new ThumbnailDelegate(this));

    m_splitter->addWidget(m_galleryView);
    
    // --- 3. Logik Verbinden ---
    connect(m_treeView, &QTreeView::clicked, this, &MainWindow::onFolderSelected);
    
    // Splitter Verhältnis setzen (30% links, 70% rechts)
    m_splitter->setSizes(QList<int>{300, 900});
}

void MainWindow::onFolderSelected(const QModelIndex& index) {
    // 1. Pfad holen und bereinigen (WICHTIG!)
    QString rawPath = m_fileSystemModel->filePath(index);
    QString path = QDir::cleanPath(rawPath);
    
    m_statusLabel->setText(tr("Scanning: ") + path);

    // 2. Indexer starten (mit dem bereinigten Pfad)
    ImageIndexer* indexer = new ImageIndexer(path);
    indexer->setAutoDelete(true);
    connect(indexer, &ImageIndexer::finished, this, &MainWindow::onIndexerFinished);
    QThreadPool::globalInstance()->start(indexer);

    // 3. Filter Variable definieren (Hier fehlte die Zeile vorhin)
    // Wir escapen einfache Hochkommas, damit SQL nicht abstürzt (z.B. "Bob's Photos")
    QString dbFilter = QString("folder_path = '%1'").arg(path.replace("'", "''"));
    
    // 4. Filter auf das Model anwenden
    m_galleryModel->setFilter(dbFilter);
    
    // 5. Daten neu laden
    if (!m_galleryModel->select()) {
        qDebug() << "Model Select Failed:" << m_galleryModel->lastError().text();
    }
    
    // Debug Ausgabe zur Kontrolle
    qDebug() << "Ordner gewählt:" << path << "| Filter:" << dbFilter << "| Gefunden:" << m_galleryModel->rowCount();
}

void MainWindow::onIndexerFinished() {
    m_statusLabel->setText(tr("Ready."));
    // Model aktualisieren, um neu gefundene Bilder anzuzeigen
    m_galleryModel->select();
}

void MainWindow::loadLanguage(const QString& lang) {
    // Implementierung analog zu deinem vorherigen Code
}