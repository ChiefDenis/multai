#include "ToolbarIcons.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QtMath>

namespace ToolbarIcons {

namespace {

QColor normalColor(bool isDark) {
  return isDark ? QColor("#E0E0E0") : QColor("#212529");
}

QColor disabledColor(bool isDark) {
  return isDark ? QColor("#5E5C64") : QColor("#ADB5BD");
}

QPixmap drawBackPixmap(int size, const QColor& color) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPen pen(color);
  pen.setWidthF(qMax(1.5, size * 0.11));
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);

  QPainterPath path;
  path.moveTo(size * 0.62, size * 0.22);
  path.lineTo(size * 0.32, size * 0.50);
  path.lineTo(size * 0.62, size * 0.78);
  painter.drawPath(path);

  return pixmap;
}

QPixmap drawForwardPixmap(int size, const QColor& color) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPen pen(color);
  pen.setWidthF(qMax(1.5, size * 0.11));
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);

  QPainterPath path;
  path.moveTo(size * 0.38, size * 0.22);
  path.lineTo(size * 0.68, size * 0.50);
  path.lineTo(size * 0.38, size * 0.78);
  painter.drawPath(path);

  return pixmap;
}

QPixmap drawFolderPixmap(int size, const QColor& color) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPen pen(color);
  pen.setWidthF(qMax(1.5, size * 0.08));
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);

  // Folder tab + body
  QPainterPath folder;
  folder.moveTo(size * 0.16, size * 0.32);
  folder.lineTo(size * 0.16, size * 0.24);
  folder.lineTo(size * 0.40, size * 0.24);
  folder.lineTo(size * 0.48, size * 0.32);
  folder.lineTo(size * 0.84, size * 0.32);
  folder.lineTo(size * 0.84, size * 0.76);
  folder.lineTo(size * 0.16, size * 0.76);
  folder.closeSubpath();
  painter.drawPath(folder);

  // Plus sign in the middle
  const qreal cx = size * 0.50;
  const qreal cy = size * 0.54;
  const qreal arm = size * 0.12;
  painter.drawLine(QPointF(cx - arm, cy), QPointF(cx + arm, cy));
  painter.drawLine(QPointF(cx, cy - arm), QPointF(cx, cy + arm));

  return pixmap;
}

QPixmap drawTabsPixmap(int size, const QColor& color) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPen pen(color);
  pen.setWidthF(qMax(1.5, size * 0.09));
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);

  // Back tab
  painter.drawRoundedRect(QRectF(size * 0.28, size * 0.18, size * 0.54, size * 0.52), size * 0.08, size * 0.08);

  // Front tab
  painter.setBrush(Qt::NoBrush);
  painter.drawRoundedRect(QRectF(size * 0.16, size * 0.30, size * 0.54, size * 0.52), size * 0.08, size * 0.08);

  return pixmap;
}

QPixmap drawSettingsPixmap(int size, const QColor& color) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPen pen(color);
  pen.setWidthF(qMax(1.5, size * 0.09));
  pen.setCapStyle(Qt::RoundCap);
  painter.setPen(pen);

  const QPointF center(size * 0.5, size * 0.5);
  painter.drawEllipse(center, size * 0.26, size * 0.26);
  painter.drawEllipse(center, size * 0.10, size * 0.10);

  for (int i = 0; i < 8; ++i) {
    const qreal angle = i * M_PI / 4.0;
    const QPointF from(center.x() + std::cos(angle) * size * 0.30,
                       center.y() + std::sin(angle) * size * 0.30);
    const QPointF to(center.x() + std::cos(angle) * size * 0.40,
                     center.y() + std::sin(angle) * size * 0.40);
    painter.drawLine(from, to);
  }

  return pixmap;
}

QPixmap drawClosePixmap(int size, const QColor& color) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPen pen(color);
  pen.setWidthF(qMax(1.5, size * 0.12));
  pen.setCapStyle(Qt::RoundCap);
  painter.setPen(pen);

  painter.drawLine(QPointF(size * 0.28, size * 0.28), QPointF(size * 0.72, size * 0.72));
  painter.drawLine(QPointF(size * 0.72, size * 0.28), QPointF(size * 0.28, size * 0.72));

  return pixmap;
}

template <typename Func>
QIcon buildIcon(Func drawFunc, bool isDark) {
  QIcon icon;
  for (int s : {16, 20, 24, 32, 48, 64}) {
    icon.addPixmap(drawFunc(s, normalColor(isDark)), QIcon::Normal);
    icon.addPixmap(drawFunc(s, disabledColor(isDark)), QIcon::Disabled);
  }
  return icon;
}

}  // namespace

QIcon backIcon(bool isDark) {
  return buildIcon(drawBackPixmap, isDark);
}

QIcon forwardIcon(bool isDark) {
  return buildIcon(drawForwardPixmap, isDark);
}

QIcon folderIcon(bool isDark) {
  return buildIcon(drawFolderPixmap, isDark);
}

QIcon tabsIcon(bool isDark) {
  return buildIcon(drawTabsPixmap, isDark);
}

QIcon settingsIcon(bool isDark) {
  return buildIcon(drawSettingsPixmap, isDark);
}

QIcon closeIcon(bool isDark) {
  return buildIcon(drawClosePixmap, isDark);
}

}  // namespace ToolbarIcons
