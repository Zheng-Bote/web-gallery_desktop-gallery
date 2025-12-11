#pragma once

#include <QList>
#include <QMainWindow>
#include <QString>

class QWebEngineView;

class MapWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MapWindow(QWidget *parent = nullptr);

  // Hauptfunktion: Nimmt Dateipfade entgegen und baut die Karte
  void showImages(const QList<QString> &filePaths);

private:
  QWebEngineView *m_webView;

  // Helper, um HTML zu generieren
  QString generateHtml(const QList<QString> &filePaths);
};