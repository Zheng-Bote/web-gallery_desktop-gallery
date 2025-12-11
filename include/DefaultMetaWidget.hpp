#pragma once

#include <QCloseEvent>
#include <QResizeEvent>
#include <QTableWidget> // Important
#include <QTableWidgetItem>
#include <QWidget>

#include "rz_metadata.hpp"

namespace Ui {
class DefaultMeta;
}

/**
 * @class DefaultMetaWidget
 * @brief Widget for configuring default metadata values.
 *
 * Allows the user to set default EXIF, IPTC, and XMP tags that can be
 * applied to images in bulk.
 */
class DefaultMetaWidget : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent The parent widget.
   */
  explicit DefaultMetaWidget(QWidget *parent = nullptr);
  ~DefaultMetaWidget();

  /**
   * @brief Set the pointer to the metadata structure to be edited.
   * @param _defaultMetaPtr Pointer to the DefaultMetaStruct.
   */
  void setDefaultMeta(RzMetadata::DefaultMetaStruct *_defaultMetaPtr);

private slots:
  void on_close_Btn_clicked();
  void on_save_Btn_clicked();

  /**
   * @brief Generic slot for handling double-clicks on table items.
   * @param item The item that was double-clicked.
   */
  void onItemDoubleClicked(QTableWidgetItem *item);

protected:
  void resizeEvent(QResizeEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

private:
  Ui::DefaultMeta *ui;

  // GUI elements for the tabs
  QTableWidget *exifTable;
  QTableWidget *iptcTable;
  QTableWidget *xmpTable;

  void setupTabs();
  void initTable(QTableWidget *table, const QMap<QString, QString> &defaults,
                 const QHash<QString, QString> &descriptions);

  void loadDataToTable(QTableWidget *table,
                       const QHash<QString, QString> &data);
  void saveDataFromTable(QTableWidget *table,
                         QHash<QString, QString> &targetData);

  bool dataModified_bool{false};
  RzMetadata::DefaultMetaStruct *defaultMetaPtr{nullptr};

  /**
   * @brief Helper function to synchronise metadata updates across tables.
   * @param targetTable The table to update.
   * @param targetKey The key to search for.
   * @param newValue The new value to set.
   */
  void updateSyncTarget(QTableWidget *targetTable, const QString &targetKey,
                        const QString &newValue);
};