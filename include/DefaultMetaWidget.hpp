#pragma once

#include <QWidget>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QTableWidget> // Wichtig

#include "rz_metadata.hpp"

namespace Ui {
class DefaultMeta; 
}

class DefaultMetaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DefaultMetaWidget(QWidget *parent = nullptr);
    ~DefaultMetaWidget();

    void setDefaultMeta(RzMetadata::DefaultMetaStruct *_defaultMetaPtr);

private slots:
    void on_close_Btn_clicked();
    void on_save_Btn_clicked();
    
    // Generischer Slot für Doppelklicks
    void onItemDoubleClicked(QTableWidgetItem *item);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::DefaultMeta *ui;

    // GUI Elemente für die Tabs
    QTableWidget *exifTable;
    QTableWidget *iptcTable;
    QTableWidget *xmpTable;

    void setupTabs();
    void initTable(QTableWidget* table, const QMap<QString, QString>& defaults, const QHash<QString, QString>& descriptions);
    
    void loadDataToTable(QTableWidget* table, const QHash<QString, QString>& data);
    void saveDataFromTable(QTableWidget* table, QHash<QString, QString>& targetData);

    bool dataModified_bool{false};
    RzMetadata::DefaultMetaStruct *defaultMetaPtr{nullptr};

    // Hilfsfunktion zum Synchronisieren
    void updateSyncTarget(QTableWidget* targetTable, const QString& targetKey, const QString& newValue);
};