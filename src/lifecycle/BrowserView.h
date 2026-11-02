#pragma once

#include <QWebEngineView>

class BrowserView : public QWebEngineView {
  Q_OBJECT
public:
  explicit BrowserView(QWidget* parent = nullptr);
  ~BrowserView() override = default;
};
