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

QPixmap providerPixmap(const QString& providerId, int size) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

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
  painter.end();

  return pixmap;
}

QIcon providerIcon(const QString& providerId) {
  QIcon icon;
  for (int size : {16, 24, 32, 48, 64, 128}) {
    icon.addPixmap(providerPixmap(providerId, size));
  }
  return icon;
}
