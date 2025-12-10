#pragma once
#include <QMainWindow>
#include <QStandardItemModel>
#include <QSqlTableModel>
#include <QTreeView>
#include <QListView>
#include <QSplitter>
#include <QLabel>
#include <QFutureWatcher>
#include <QTranslator>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>

#include "rz_metadata.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadLanguage(const QString& lang);

private slots:
    void openSrcFolder();           
    void openSrcFolderRekursive();  
    
    void onFolderClicked(const QModelIndex& index); 
    void onGalleryDoubleClicked(const QModelIndex& index);
    void onIndexerFinished();

    void showContextMenu(const QPoint& pos);
    void exportSelectedToWebP();
    
    void showDefaultMetaWidget();
    void onImageChanged(const QString& path);

    // --- Actions ---
    void selectAllImages();
    void renameSelectedImages();

    void writeDefaultCopyrightToSelection();
    void writeDefaultGpsToSelection();
    void writeAllDefaultsToSelection();

    void setCopyrightAsDefaultFromCurrent();
    void setGpsAsDefaultFromCurrent();
    void setAllAsDefaultFromCurrent();
    
    void updateDbMetadata(const QString& path);

    // --- GEONAMES / SETTINGS ---
    void openSettingsDialog();
    void startGeoNamesLookup();
    void onGeoNamesReply(QNetworkReply* reply);

private:
    void setupUi();
    void setupDatabase();
    void createMenu(); 
    
    void addFolderToTree(const QString& path, bool recursive);
    void addSubfoldersRecursively(const QString& path, QStandardItem* parentItem);
    
    QList<QString> getSelectedFilePaths();

    QSplitter* m_splitter;
    QTreeView* m_treeView;
    QStandardItemModel* m_folderModel; 

    QListView* m_galleryView;
    QSqlTableModel* m_galleryModel;
    
    QLabel* m_statusLabel;
    
    RzMetadata::DefaultMetaStruct defaultMeta;

    QTranslator m_translator;
    QTranslator m_translatorQt;
    QString m_currLang;

    // View Options
    QAction* actShowCopyright;
    QAction* actShowGps;

    // WebP Options
    QAction* actWebpOversize;
    QAction* actWebpOverwrite;
    QAction* actWebpWatermark;
    QAction* actWebpRename;

    // Helper um den nächsten Request in der Queue zu starten
    void processNextGeoLookup();

    // --- NETZWERK & SETTINGS ---
    QSettings* m_settings;
    QNetworkAccessManager* m_netManager;
    
    // Warteschlange: Speichert Pfade, die noch verarbeitet werden müssen
    QQueue<QString> m_geoLookupQueue;
    
    // Fortschrittsanzeige für den Batch-Vorgang
    int m_geoTotalCount{0};
};