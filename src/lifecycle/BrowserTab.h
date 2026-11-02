#pragma once

#include <QTimer>
#include <QUrl>
#include <QWebEnginePage>
#include <QWidget>
#include "BrowserView.h"
#include "LifecycleTarget.h"
#include "Profile.h"
#include "TabMetadata.h"

class LifecycleManager;

class BrowserTab : public QWidget, public LifecycleTarget {
  Q_OBJECT
public:
  BrowserTab(const QString& providerId, const QString& profileId, const QUrl& url,
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

  void activate() override;
  void deactivate() override;
  void freeze() override;
  void hibernate() override;
  void closeTab();

signals:
  void lifecycleStateChanged(LifecycleState state);
  void loadStarted();
  void loadFinished(bool success);
  void titleChanged(const QString& title);
  void renderProcessTerminated(int status, int exitCode);

private slots:
  void onQtLifecycleStateChanged(QWebEnginePage::LifecycleState qtState);
  void onRecommendedStateChanged(QWebEnginePage::LifecycleState recommendedState);
  void onLoadStarted();
  void onLoadFinished(bool success);
  void onTitleChanged(const QString& title);
  void onRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);

private:
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
};
