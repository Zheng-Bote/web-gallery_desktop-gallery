#pragma once

#include "LoginDialog.hpp"
#include "MapWindow.hpp"
#include "UploadManager.hpp"

#include <QEvent> // Important for changeEvent
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

/**
 * @class MainWindow
 * @brief The main application window.
 *
 * Coordinates the folder tree, image gallery, metadata operations, and
 * settings. Acts as the central controller for the application.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent The parent widget.
   */
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  /**
   * @brief Load and apply a specific language.
   * @param lang The language code (e.g., "en", "de").
   */
  void loadLanguage(const QString &lang);

protected:
  /**
   * @brief Event handler for state changes.
   * @param event The event object.
   *
   * NEW: Reacts to language change events.
   */
  void changeEvent(QEvent *event) override;

private slots:
  void openSrcFolder();
  void openSrcFolderRekursive();

  void showAboutDialog(); // NEW

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

  void showMapForSelection();

  // --- GEONAMES / SETTINGS ---
  void openSettingsDialog();
  void startGeoNamesLookup();
  void onGeoNamesReply(QNetworkReply *reply);

private:
  void setupUi();
  void setupDatabase();
  void createMenu();

  /**
   * @brief Retranslate the UI elements.
   *
   * NEW: Resets texts after a language change.
   */
  void retranslateUi();

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

  // Helper to start the next request in the queue
  void processNextGeoLookup();

  // --- NETWORK & SETTINGS ---
  QSettings *m_settings;
  QNetworkAccessManager *m_netManager;

  // Queue: Stores paths that still need to be processed
  QQueue<QString> m_geoLookupQueue;

  // Progress indicator for the batch process
  int m_geoTotalCount{0};

  // --- GUI MEMBERS (NEW: Must be members for retranslateUi) ---

  // Menus
  QMenu *fileMenu;
  QMenu *langMenu; // NEW
  QMenu *settingsMenu;
  QMenu *metaMenu;
  QMenu *picMenu;
  QMenu *viewMenu;
  QMenu *webpMenu;
  QMenu *helpMenu; // NEW

  // File Actions
  QAction *openAct;
  QAction *openRecAct;
  QAction *exitAct;
  QAction *actLangEn; // NEW
  QAction *actLangDe; // NEW

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
  QAction *actViewMap;

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
  QAction *aboutAct; // NEW

  // Uploads
  UploadManager *m_uploader;
  QProgressDialog *m_uploadProgressDlg;
};