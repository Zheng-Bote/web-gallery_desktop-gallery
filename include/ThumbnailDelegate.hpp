#pragma once
#include <QPainter>
#include <QStyledItemDelegate>

/**
 * @class ThumbnailDelegate
 * @brief Custom item delegate for rendering image thumbnails.
 *
 * This delegate is responsible for drawing the image grid items, including
 * thumbnails, borders, text overlays (like "Copyright" or "GPS"), and selection
 * states.
 */
class ThumbnailDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent The parent object.
   */
  explicit ThumbnailDelegate(QObject *parent = nullptr);

  /**
   * @brief Set whether to show the copyright overlay.
   * @param show true to show, false to hide.
   */
  void setShowCopyright(bool show) { m_showCopyright = show; }

  /**
   * @brief Set whether to show the GPS overlay.
   * @param show true to show, false to hide.
   */
  void setShowGps(bool show) { m_showGps = show; }

  /**
   * @brief Paint the item.
   * @param painter The painter to use.
   * @param option The style options.
   * @param index The model index of the item.
   */
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;

  /**
   * @brief Provide size hints for the item.
   * @param option The style options.
   * @param index The model index.
   * @return The size hint.
   */
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;

private:
  bool m_showCopyright{false};
  bool m_showGps{false};
};