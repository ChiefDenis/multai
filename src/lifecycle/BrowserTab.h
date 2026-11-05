#pragma once

#include <QTimer>
#include <QUrl>
#include <QWebEnginePage>
#include <QWidget>
#include "BrowserView.h"
#include "LifecycleTarget.h"
#include "Profile.h"
#include "TabMetadata.h"

class QLabel;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;
class LifecycleManager;

class BrowserTab : public QWidget, public LifecycleTarget {
  Q_OBJECT
public:
  BrowserTab(const QString& providerId, const QString& profileId, const QUrl& url,
             LifecycleState initialState = LifecycleState::Unloaded,
             QWidget* parent = nullptr);
  ~BrowserTab() override;

  LifecycleState lifecycleState() const override { return m_metadata.lifecycleState(); }
  bool isSuspended() const override;
  bool isSuspendable() const override;

  QString providerId() const { return m_metadata.providerId(); }
  QString profileId() const { return m_metadata.profileId(); }
  QUrl url() const { return m_metadata.url(); }
  QString title() const { return m_metadata.title(); }
  TabMetadata metadata() const { return m_metadata; }

  bool hasError() const { return m_hasError; }
  QString errorTitle() const { return m_errorTitle; }
  QString errorMessage() const { return m_errorMessage; }
  QWidget* errorWidget() const { return m_errorWidget; }

  void activate() override;
  void deactivate() override;
  void freeze() override;
  void hibernate() override;
  void closeTab();

public slots:
  void retry();
  void onLoadStarted();
  void onLoadFinished(bool success);
  void onTitleChanged(const QString& title);
  void onRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);

signals:
  void lifecycleStateChanged(LifecycleState state);
  void loadStarted();
  void loadFinished(bool success);
  void titleChanged(const QString& title);
  void renderProcessTerminated(int status, int exitCode);
  void errorChanged(bool hasError);
  void closeRequested();

private slots:
  void onQtLifecycleStateChanged(QWebEnginePage::LifecycleState qtState);
  void onRecommendedStateChanged(QWebEnginePage::LifecycleState recommendedState);

private:
  void setupUi();
  void setupErrorUi();
  void showError(const QString& title, const QString& message);
  void hideError();
  void createPage();
  void destroyPage();
  void setQtLifecycleState(QWebEnginePage::LifecycleState state);
  void setLifecycleState(LifecycleState state);
  void scheduleFreeze();
  void scheduleHibernate();
  bool maySuspendTo(QWebEnginePage::LifecycleState target) const;

  TabMetadata m_metadata;
  Profile* m_profile;
  BrowserView* m_view;
  QWebEnginePage* m_page;
  QTimer* m_freezeTimer;
  QTimer* m_hibernateTimer;
  QWebEnginePage::LifecycleState m_qtLifecycleState;
  QWebEnginePage::LifecycleState m_recommendedState;
  bool m_pageCreated;
  bool m_recoveryAttempted;

  QStackedWidget* m_stack;
  QWidget* m_viewContainer;
  QVBoxLayout* m_viewLayout;
  QWidget* m_errorWidget;
  QLabel* m_errorIconLabel;
  QLabel* m_errorTitleLabel;
  QLabel* m_errorMessageLabel;
  QPushButton* m_retryButton;
  QPushButton* m_closeButton;
  bool m_hasError;
  QString m_errorTitle;
  QString m_errorMessage;
};
