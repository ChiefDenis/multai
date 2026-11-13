#pragma once

#include <QIcon>
#include <QPixmap>
#include <QString>
#include "TabMetadata.h"

QPixmap providerPixmap(const QString& providerId, int size,
                       LifecycleState state = LifecycleState::Active,
                       bool hasError = false);
QIcon providerIcon(const QString& providerId,
                   LifecycleState state = LifecycleState::Active,
                   bool hasError = false);
