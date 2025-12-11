#pragma once

#include "LoginDialog.hpp"
#include "UploadManager.hpp"

#include <QEvent> // Wichtig für changeEvent
#include <QFutureWatcher>
#include <QLabel>
#include <QListView>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QQueue>
#include <QSettings>
#include <QSplitter>
#include <QSqlTableModel>
#include <QStandardItemModel>
#include <QTranslator>
#include <QTreeView>

#include "rz_metadata.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void loadLanguage(const QString &lang);

protected:
  // NEU: Reagiert auf Sprachwechsel-Events
  void changeEvent(QEvent *event) override;

private slots:
  void openSrcFolder();
  void openSrcFolderRekursive();

  void showAboutDialog(); // NEU

  void onFolderClicked(const QModelIndex &index);
  void onGalleryDoubleClicked(const QModelIndex &index);
  void onIndexerFinished();

  void showContextMenu(const QPoint &pos);
  void exportSelectedToWebP();

  void showDefaultMetaWidget();
  void onImageChanged(const QString &path);

  // --- Actions ---
  void selectAllImages();
  void renameSelectedImages();

  void writeDefaultCopyrightToSelection();
  void writeDefaultGpsToSelection();
  void writeAllDefaultsToSelection();

  void setCopyrightAsDefaultFromCurrent();
  void setGpsAsDefaultFromCurrent();
  void setAllAsDefaultFromCurrent();

  void updateDbMetadata(const QString &path);

  void uploadSelectedImages();

  // --- GEONAMES / SETTINGS ---
  void openSettingsDialog();
  void startGeoNamesLookup();
  void onGeoNamesReply(QNetworkReply *reply);

private:
  void setupUi();
  void setupDatabase();
  void createMenu();
  void retranslateUi(); // NEU: Setzt Texte neu

  void addFolderToTree(const QString &path, bool recursive);
  void addSubfoldersRecursively(const QString &path, QStandardItem *parentItem);

  QList<QString> getSelectedFilePaths();

  QSplitter *m_splitter;
  QTreeView *m_treeView;
  QStandardItemModel *m_folderModel;

  QListView *m_galleryView;
  QSqlTableModel *m_galleryModel;

  QLabel *m_statusLabel;

  RzMetadata::DefaultMetaStruct defaultMeta;

  QTranslator m_translator;
  QTranslator m_translatorQt;
  QString m_currLang;

  // Helper um den nächsten Request in der Queue zu starten
  void processNextGeoLookup();

  // --- NETZWERK & SETTINGS ---
  QSettings *m_settings;
  QNetworkAccessManager *m_netManager;

  // Warteschlange: Speichert Pfade, die noch verarbeitet werden müssen
  QQueue<QString> m_geoLookupQueue;

  // Fortschrittsanzeige für den Batch-Vorgang
  int m_geoTotalCount{0};

  // --- GUI MEMBERS (NEU: Müssen Member sein für retranslateUi) ---

  // Menüs
  QMenu *fileMenu;
  QMenu *langMenu; // NEU
  QMenu *settingsMenu;
  QMenu *metaMenu;
  QMenu *picMenu;
  QMenu *viewMenu;
  QMenu *webpMenu;
  QMenu *helpMenu; // NEU

  // File Actions
  QAction *openAct;
  QAction *openRecAct;
  QAction *exitAct;
  QAction *actLangEn; // NEU
  QAction *actLangDe; // NEU

  // Settings Actions
  QAction *geoUserAct;

  // Metadata Actions
  QAction *metaAct;
  QAction *writeCpAct;
  QAction *writeGpsAct;
  QAction *writeAllAct;
  QAction *geoLookupAct;

  // Picture Actions
  QAction *actSelectAll;
  QAction *actRename;

  // View Options
  QAction *actShowCopyright;
  QAction *actShowGps;

  // WebP Options
  QAction *actWebpOversize;
  QAction *actWebpOverwrite;
  QAction *actWebpWatermark;
  QAction *actWebpRename;
  QAction *actExportWebp;

  // Help Actions
  QAction *aboutAct; // NEU

  // Uploads
  UploadManager *m_uploader;
  QProgressDialog *m_uploadProgressDlg;
};