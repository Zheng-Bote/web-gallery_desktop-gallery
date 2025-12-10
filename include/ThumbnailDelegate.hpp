#pragma once
#include <QStyledItemDelegate>
#include <QPainter>

class ThumbnailDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ThumbnailDelegate(QObject *parent = nullptr);

    // Setter für die Optionen
    void setShowCopyright(bool show) { m_showCopyright = show; }
    void setShowGps(bool show) { m_showGps = show; }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    bool m_showCopyright{false};
    bool m_showGps{false};
};