#pragma once

#include <QDialog>
#include <QPainter>

// QDialog does not paint a stylesheet or palette background on Wayland when
// captured through QWidget::grab(), leaving an unthemed surface behind custom
// children. Application-owned dialogs derive from this class so the window
// background is always filled with the active theme colour.
class ThemedDialog : public QDialog {
public:
  explicit ThemedDialog(QWidget* parent = nullptr)
    : QDialog(parent) {
  }

protected:
  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    painter.fillRect(rect(), palette().color(QPalette::Window));
    QDialog::paintEvent(event);
  }
};
