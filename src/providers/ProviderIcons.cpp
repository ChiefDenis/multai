#include "ProviderIcons.h"

#include <QDebug>
#include <QGuiApplication>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPalette>
#include <QPixmap>

void aiWorkspaceInitProviderLogos();

namespace {

constexpr int kLogoInsetPercent = 8;
const int kIconSizes[] = {16, 20, 24, 28, 32, 42, 48, 64, 128};

const QHash<QString, QString>& logoFiles() {
  static const QHash<QString, QString> files{
      {"deepseek", "deepseek.svg"},
      {"qwen", "qwen.svg"},
      {"glm", "glm.svg"},
      {"copilot", "copilot.svg"},
      {"gemini", "gemini.svg"},
      {"claude", "claude.svg"},
      {"perplexity", "perplexity.svg"},
      {"chatgpt", "chatgpt.svg"},
      {"kimi", "kimi.svg"},
  };
  return files;
}

bool isMonochrome(const QString& providerId) {
  return providerId == "chatgpt" || providerId == "claude" || providerId == "kimi";
}

bool isDarkTheme() {
  return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
}

void ensureLogosRegistered() {
  static const bool registered = []() {
    aiWorkspaceInitProviderLogos();
    return true;
  }();
  Q_UNUSED(registered)
}

QString logoPath(const QString& providerId) {
  const QString file = logoFiles().value(providerId);
  return file.isEmpty() ? QString() : QStringLiteral(":/provider-logos/") + file;
}

QImage renderLogo(const QString& providerId, int boxSize, bool dark) {
  ensureLogosRegistered();

  const QString path = logoPath(providerId);
  if (path.isEmpty()) {
    qWarning() << "[ProviderIcons] No logo asset registered for provider:" << providerId;
    return {};
  }

  QImageReader reader(path);
  reader.setAutoTransform(true);
  const QSize intrinsic = reader.size();
  if (!intrinsic.isValid() || intrinsic.isEmpty()) {
    qWarning() << "[ProviderIcons] Cannot read logo dimensions for provider:" << providerId
               << reader.errorString();
    return {};
  }

  const int inset = qMax(1, boxSize * kLogoInsetPercent / 100);
  const int content = qMax(1, boxSize - 2 * inset);
  const QSize scaled = intrinsic.scaled(content, content, Qt::KeepAspectRatio);
  reader.setScaledSize(scaled);

  QImage image = reader.read();
  if (image.isNull()) {
    qWarning() << "[ProviderIcons] Failed to render logo for provider:" << providerId
               << reader.errorString();
    return {};
  }
  image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

  if (isMonochrome(providerId)) {
    QPainter painter(&image);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(image.rect(), dark ? QColor("#f2f2f2") : QColor("#161616"));
  }

  return image;
}

}  // namespace

QPixmap providerPixmap(const QString& providerId, int size, LifecycleState state,
                       bool hasError) {
  Q_UNUSED(state)
  Q_UNUSED(hasError)

  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  const QImage logo = renderLogo(providerId, size, isDarkTheme());
  if (logo.isNull()) {
    return pixmap;
  }

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter.drawImage((size - logo.width()) / 2, (size - logo.height()) / 2, logo);
  return pixmap;
}

QIcon providerIcon(const QString& providerId, LifecycleState state, bool hasError) {
  QIcon icon;
  for (int size : kIconSizes) {
    icon.addPixmap(providerPixmap(providerId, size, state, hasError));

    QPixmap highDpi = providerPixmap(providerId, size * 2, state, hasError);
    highDpi.setDevicePixelRatio(2.0);
    icon.addPixmap(highDpi);
  }
  return icon;
}
