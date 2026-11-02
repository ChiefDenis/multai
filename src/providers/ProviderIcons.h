#pragma once

#include <QColor>
#include <QIcon>
#include <QString>

QColor providerColor(const QString& providerId);
QString providerInitial(const QString& providerId);
QPixmap providerPixmap(const QString& providerId, int size);
QIcon providerIcon(const QString& providerId);
