#include "DefaultMetaWidget.hpp"
#include "ui_DefaultMetaWidget.h" 

#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHeaderView>

DefaultMetaWidget::DefaultMetaWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DefaultMeta)
{
    ui->setupUi(this);
    setupTabs();
}

DefaultMetaWidget::~DefaultMetaWidget()
{
    delete ui;
}

void DefaultMetaWidget::setupTabs() {
    // FIX: Variable umbenannt zu 'mainLayout', um Konflikt mit Methode layout() zu vermeiden
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    
    if (!mainLayout) {
        mainLayout = new QVBoxLayout(this);
    }

    QTabWidget* tabWidget = new QTabWidget(this);
    
    exifTable = new QTableWidget(this);
    iptcTable = new QTableWidget(this);
    xmpTable = new QTableWidget(this);

    initTable(exifTable, RzMetadata::exifDefaultTagsList, RzMetadata::exifTagDescriptions);
    initTable(iptcTable, RzMetadata::iptcDefaultTagsList, RzMetadata::iptcTagDescriptions);
    initTable(xmpTable, RzMetadata::xmpDefaultTagsList, RzMetadata::xmpTagDescriptions);

    tabWidget->addTab(exifTable, "EXIF");
    tabWidget->addTab(iptcTable, "IPTC");
    tabWidget->addTab(xmpTable, "XMP");

    // TabWidget an erster Stelle einfügen
    mainLayout->insertWidget(0, tabWidget);
    
    // Alte Platzhalter im UI File verstecken (falls vorhanden)
    if (ui->tableWidget) ui->tableWidget->hide(); 
    if (ui->exif_tableWidget) ui->exif_tableWidget->hide();
}

void DefaultMetaWidget::initTable(QTableWidget* table, const QMap<QString, QString>& defaults, const QHash<QString, QString>& descriptions) {
    QFont font_bold("Times New Roman", 11);
    font_bold.setBold(true);

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setColumnCount(3);
    table->setRowCount(0);
    
    connect(table, &QTableWidget::itemDoubleClicked, this, &DefaultMetaWidget::onItemDoubleClicked);

    QStringList headers = {tr("Key"), tr("Value"), tr("Description")};
    table->setHorizontalHeaderLabels(headers);
    
    for (int i = 0; i < 3; ++i) {
        if(table->horizontalHeaderItem(i)) {
            table->horizontalHeaderItem(i)->setBackground(Qt::lightGray);
            table->horizontalHeaderItem(i)->setFont(font_bold);
        }
    }

    for (auto i = defaults.cbegin(), end = defaults.cend(); i != end; ++i) {
        int row = table->rowCount();
        table->insertRow(row);

        QTableWidgetItem *keyItem = new QTableWidgetItem(i.key());
        keyItem->setFlags(keyItem->flags() ^ Qt::ItemIsEditable);
        table->setItem(row, 0, keyItem);

        // KORREKTUR: Hier stand vorher "", jetzt nehmen wir i.value()
        // Damit werden die Werte aus rz_metadata.hpp geladen.
        QTableWidgetItem *valItem = new QTableWidgetItem(i.value()); 
        table->setItem(row, 1, valItem);

        QString desc = descriptions.value(i.key(), "");
        QTableWidgetItem *descItem = new QTableWidgetItem(desc);
        descItem->setFlags(descItem->flags() ^ Qt::ItemIsEditable);
        table->setItem(row, 2, descItem);
    }
    
    table->resizeColumnsToContents();
}

void DefaultMetaWidget::setDefaultMeta(RzMetadata::DefaultMetaStruct *_defaultMetaPtr)
{
    defaultMetaPtr = _defaultMetaPtr;
    if (!defaultMetaPtr) return;

    loadDataToTable(exifTable, defaultMetaPtr->exifDefault);
    loadDataToTable(iptcTable, defaultMetaPtr->iptcDefault);
    loadDataToTable(xmpTable, defaultMetaPtr->xmpDefault);
}

void DefaultMetaWidget::loadDataToTable(QTableWidget* table, const QHash<QString, QString>& data) {
    QHashIterator<QString, QString> it(data);
    while (it.hasNext()) {
        it.next();
        QString key = it.key();
        QString val = it.value();

        auto list = table->findItems(key, Qt::MatchExactly);
        
        if (list.isEmpty()) {
            // Custom Key hinzufügen
            int row = table->rowCount();
            table->insertRow(row);
            
            QTableWidgetItem *keyItem = new QTableWidgetItem(key);
            keyItem->setFlags(keyItem->flags() ^ Qt::ItemIsEditable);
            table->setItem(row, 0, keyItem);
            
            table->setItem(row, 1, new QTableWidgetItem(val));
            table->setItem(row, 2, new QTableWidgetItem("Custom Key"));
        } else {
            for (auto item : list) {
                if (item->column() == 0) {
                    if (table->item(item->row(), 1)) {
                        table->item(item->row(), 1)->setText(val);
                    }
                }
            }
        }
    }
}

void DefaultMetaWidget::onItemDoubleClicked(QTableWidgetItem *item)
{
    QTableWidget* senderTable = qobject_cast<QTableWidget*>(sender());
    if (!senderTable) return;

    int row = item->row();
    
    QString key = senderTable->item(row, 0)->text();
    QString currentVal = "";
    if (senderTable->item(row, 1)) {
        currentVal = senderTable->item(row, 1)->text();
    }

    bool ok;
    QString text = QInputDialog::getText(this,
                                         tr("Edit Metadata"),
                                         tr("Value for") + " " + key + ":",
                                         QLineEdit::Normal,
                                         currentVal,
                                         &ok);

    if (ok) {
        dataModified_bool = true;
        
        // 1. Wert in der aktuellen Tabelle setzen
        if (!senderTable->item(row, 1)) {
            senderTable->setItem(row, 1, new QTableWidgetItem(text));
        } else {
            senderTable->item(row, 1)->setText(text);
        }
        senderTable->resizeColumnsToContents();

        // 2. --- SYNCHRONISATION ---
        
        // Fall A: Änderung im EXIF Tab
        if (senderTable == exifTable) {
            // Exif -> XMP
            if (RzMetadata::exif_to_xmp.contains(key)) {
                updateSyncTarget(xmpTable, RzMetadata::exif_to_xmp[key], text);
            }
            // Exif -> IPTC
            if (RzMetadata::exif_to_iptc.contains(key)) {
                updateSyncTarget(iptcTable, RzMetadata::exif_to_iptc[key], text);
            }
        }
        
        // Fall B: Änderung im IPTC Tab
        else if (senderTable == iptcTable) {
            // IPTC -> Exif
            if (RzMetadata::iptc_to_exif.contains(key)) {
                updateSyncTarget(exifTable, RzMetadata::iptc_to_exif[key], text);
            }
            // IPTC -> XMP
            if (RzMetadata::iptc_to_xmp.contains(key)) {
                updateSyncTarget(xmpTable, RzMetadata::iptc_to_xmp[key], text);
            }
        }
        
        // Fall C: Änderung im XMP Tab
        else if (senderTable == xmpTable) {
            // XMP -> Exif
            if (RzMetadata::xmp_to_exif.contains(key)) {
                updateSyncTarget(exifTable, RzMetadata::xmp_to_exif[key], text);
            }
            // XMP -> IPTC
            if (RzMetadata::xmp_to_iptc.contains(key)) {
                updateSyncTarget(iptcTable, RzMetadata::xmp_to_iptc[key], text);
            }
        }
    }
}

void DefaultMetaWidget::updateSyncTarget(QTableWidget* targetTable, const QString& targetKey, const QString& newValue) {
    if (!targetTable) return;

    // Wir suchen in der Zieltabelle nach dem Key
    auto items = targetTable->findItems(targetKey, Qt::MatchExactly);
    
    if (items.isEmpty()) {
        // Optional: Wenn der Key noch nicht existiert, könnte man ihn anlegen.
        // Da wir aber Default-Werte bearbeiten, gehen wir davon aus, dass die relevanten 
        // Felder bereits initialisiert sind. Falls du "Auto-Create" willst, hier insertRow einfügen.
        // Für jetzt: Nur update, wenn vorhanden.
    } else {
        for (auto item : items) {
            // Wir prüfen, ob es die Key-Spalte (0) ist
            if (item->column() == 0) {
                int row = item->row();
                // Value-Item in Spalte 1 holen/erstellen
                if (!targetTable->item(row, 1)) {
                    targetTable->setItem(row, 1, new QTableWidgetItem(newValue));
                } else {
                    targetTable->item(row, 1)->setText(newValue);
                }
            }
        }
        targetTable->resizeColumnsToContents();
    }
}

void DefaultMetaWidget::on_save_Btn_clicked()
{
    if (defaultMetaPtr) {
        saveDataFromTable(exifTable, defaultMetaPtr->exifDefault);
        saveDataFromTable(iptcTable, defaultMetaPtr->iptcDefault);
        saveDataFromTable(xmpTable, defaultMetaPtr->xmpDefault);
    }
    
    dataModified_bool = false;
    this->close();
}

void DefaultMetaWidget::saveDataFromTable(QTableWidget* table, QHash<QString, QString>& targetData) {
    targetData.clear();
    
    for (int i = 0; i < table->rowCount(); ++i) {
        QString key = table->item(i, 0)->text();
        QString val = "";
        if (table->item(i, 1)) {
            val = table->item(i, 1)->text();
        }
        
        if (!val.isEmpty()) {
            targetData[key] = val;
        }
    }
}

void DefaultMetaWidget::on_close_Btn_clicked()
{
    this->close();
}

void DefaultMetaWidget::closeEvent(QCloseEvent *event)
{
    if (dataModified_bool) {
        QMessageBox::StandardButton response;
        response = QMessageBox::question(this,
                                         tr("Confirmation"),
                                         tr("Unsaved changes. Save now?"),
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (response == QMessageBox::Yes) {
            on_save_Btn_clicked();
            event->accept();
        } else if (response == QMessageBox::No) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void DefaultMetaWidget::resizeEvent(QResizeEvent *event)
{
    if(exifTable) exifTable->resizeColumnsToContents();
    if(iptcTable) iptcTable->resizeColumnsToContents();
    if(xmpTable) xmpTable->resizeColumnsToContents();
    QWidget::resizeEvent(event);
}