#include "ProviderIcons.h"

#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRectF>

namespace {

struct BrandColour {
  const char* id;
  const char* colour;
  const char* initial;
};

const BrandColour kBrands[] = {
    {"deepseek", "#4D6BFE", "D"},
    {"qwen", "#615CED", "Q"},
    {"glm", "#1F6FEB", "G"},
    {"copilot", "#0078D4", "C"},
    {"gemini", "#4285F4", "G"},
    {"claude", "#D97757", "C"},
    {"perplexity", "#20808D", "P"},
    {"chatgpt", "#10A37F", "C"},
    {"kimi", "#333333", "K"},
};

const BrandColour* findBrand(const QString& providerId) {
  for (const BrandColour& brand : kBrands) {
    if (providerId == QLatin1String(brand.id)) {
      return &brand;
    }
  }
  return nullptr;
}

}  // namespace

QColor providerColor(const QString& providerId) {
  const BrandColour* brand = findBrand(providerId);
  if (brand) {
    return QColor(QLatin1String(brand->colour));
  }
  return QColor(QString("#6E6E6E"));
}

QString providerInitial(const QString& providerId) {
  const BrandColour* brand = findBrand(providerId);
  if (brand) {
    return QLatin1String(brand->initial);
  }
  return providerId.isEmpty() ? QString("?") : providerId.left(1).toUpper();
}

QPixmap providerPixmap(const QString& providerId, int size, LifecycleState state, bool hasError) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  if (state == LifecycleState::Discarded) {
    painter.setOpacity(0.60);
  }

  QPainterPath path;
  const qreal radius = size * 0.25;
  path.addRoundedRect(QRectF(0, 0, size, size), radius, radius);
  painter.fillPath(path, providerColor(providerId));

  QFont font = painter.font();
  font.setPixelSize(qMax(1, static_cast<int>(size * 0.55)));
  font.setBold(true);
  painter.setFont(font);
  painter.setPen(Qt::white);
  painter.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, providerInitial(providerId));

  painter.setOpacity(1.0);

  if (hasError || state != LifecycleState::Active) {
    QColor badgeColor;
    if (hasError) {
      badgeColor = QColor("#E01B24");
    } else {
      switch (state) {
        case LifecycleState::Frozen:
          badgeColor = QColor("#33C7DE");
          break;
        case LifecycleState::Discarded:
          badgeColor = QColor("#9A9996");
          break;
        case LifecycleState::Loading:
          badgeColor = QColor("#3584E4");
          break;
        case LifecycleState::Unloaded:
          badgeColor = QColor("#77767B");
          break;
        case LifecycleState::Closing:
        case LifecycleState::Active:
          break;
      }
    }

    if (badgeColor.isValid()) {
      const qreal badgeDiameter = qMax(4.0, size * 0.32);
      const qreal badgeRadius = badgeDiameter / 2.0;
      const qreal cx = size - badgeRadius - 1.0;
      const qreal cy = size - badgeRadius - 1.0;

      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(36, 36, 36, 180));
      painter.drawEllipse(QPointF(cx, cy), badgeRadius + 1.0, badgeRadius + 1.0);

      painter.setBrush(badgeColor);
      painter.drawEllipse(QPointF(cx, cy), badgeRadius, badgeRadius);
    }
  }

  painter.end();
  return pixmap;
}

QIcon providerIcon(const QString& providerId, LifecycleState state, bool hasError) {
  QIcon icon;
  for (int size : {16, 24, 32, 48, 64, 128}) {
    icon.addPixmap(providerPixmap(providerId, size, state, hasError));
  }
  return icon;
}
