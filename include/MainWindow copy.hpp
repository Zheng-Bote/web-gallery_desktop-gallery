#pragma once
#include <QMainWindow>
#include <QFileSystemModel>
#include <QSqlTableModel>
#include <QTreeView>
#include <QListView>
#include <QSplitter>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void loadLanguage(const QString& lang);

private slots:
    void onFolderSelected(const QModelIndex& index);
    void onIndexerFinished();

private:
    void setupUi();
    void setupDatabase();

    QSplitter* m_splitter;
    
    // Links: Ordnerstruktur
    QTreeView* m_treeView;
    QFileSystemModel* m_fileSystemModel;

    // Rechts: Galerie
    QListView* m_galleryView;
    QSqlTableModel* m_galleryModel;
    
    QLabel* m_statusLabel;
};