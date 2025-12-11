#pragma once

#include <QList>
#include <QMainWindow>
#include <QString>

class QWebEngineView;

/**
 * @class MapWindow
 * @brief Window for displaying images on a map.
 *
 * Uses a QWebEngineView to render an OpenStreetMap with markers for
 * the provided images (based on their GPS coordinates).
 */
class MapWindow : public QMainWindow {
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent The parent widget.
   */
  explicit MapWindow(QWidget *parent = nullptr);

  /**
   * @brief Main function: Takes file paths and builds the map.
   * @param filePaths List of absolute paths to the images to display.
   */
  void showImages(const QList<QString> &filePaths);

private:
  QWebEngineView *m_webView;

  /**
   * @brief Helper to generate HTML content for the map.
   * @param filePaths List of image paths.
   * @return The generated HTML string.
   */
  QString generateHtml(const QList<QString> &filePaths);
};