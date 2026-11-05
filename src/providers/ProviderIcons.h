#pragma once

#include <QColor>
#include <QIcon>
#include <QString>
#include "TabMetadata.h"

QColor providerColor(const QString& providerId);
QString providerInitial(const QString& providerId);
QPixmap providerPixmap(const QString& providerId, int size,
                       LifecycleState state = LifecycleState::Active,
                       bool hasError = false);
QIcon providerIcon(const QString& providerId,
                   LifecycleState state = LifecycleState::Active,
                   bool hasError = false);
