#pragma once

#include "TabMetadata.h"

class LifecycleTarget {
public:
  virtual ~LifecycleTarget() = default;

  virtual LifecycleState lifecycleState() const = 0;
  virtual bool isSuspended() const = 0;
  virtual bool isSuspendable() const = 0;
  virtual void activate() = 0;
  virtual void deactivate() = 0;
  virtual void freeze() = 0;
  virtual void hibernate() = 0;
};
