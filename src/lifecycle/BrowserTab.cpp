#include "BrowserTab.h"
#include "LifecycleManager.h"
#include "SettingsManager.h"
#include "profiles/ProfileManager.h"
#include <QVBoxLayout>
#include <QWebEngineProfile>

BrowserTab::BrowserTab(const QString& providerId, const QString& profileId, const QUrl& url, QWidget* parent)
  : QWidget(parent)
  , m_metadata(providerId, profileId, url)
  , m_profile(nullptr)
  , m_view(nullptr)
  , m_page(nullptr)
  , m_freezeTimer(nullptr)
  , m_hibernateTimer(nullptr)
  , m_qtLifecycleState(QWebEnginePage::LifecycleState::Active)
  , m_recommendedState(QWebEnginePage::LifecycleState::Active)
  , m_pageCreated(false)
  , m_recoveryAttempted(false) {
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  m_freezeTimer = new QTimer(this);
  m_freezeTimer->setSingleShot(true);
  connect(m_freezeTimer, &QTimer::timeout, this, &BrowserTab::freeze);

  m_hibernateTimer = new QTimer(this);
  m_hibernateTimer->setSingleShot(true);
  connect(m_hibernateTimer, &QTimer::timeout, this, &BrowserTab::hibernate);
}

BrowserTab::~BrowserTab() {
  destroyPage();
}

void BrowserTab::createPage() {
  if (m_pageCreated) {
    return;
  }

  ProfileManager* profileManager = ProfileManager::instance();
  m_profile = profileManager->getOrCreateProfile(m_metadata.providerId(), m_metadata.profileId());
  if (!m_profile) {
    return;
  }

  m_profile->ref();

  m_view = new BrowserView(this);
  m_page = new QWebEnginePage(m_profile->webEngineProfile(), m_view);
  m_view->setPage(m_page);

  m_qtLifecycleState = m_page->lifecycleState();
  m_recommendedState = m_page->recommendedState();

  connect(m_page, &QWebEnginePage::lifecycleStateChanged, this, &BrowserTab::onQtLifecycleStateChanged);
  connect(m_page, &QWebEnginePage::recommendedStateChanged, this, &BrowserTab::onRecommendedStateChanged);
  connect(m_page, &QWebEnginePage::loadStarted, this, &BrowserTab::onLoadStarted);
  connect(m_page, &QWebEnginePage::loadFinished, this, &BrowserTab::onLoadFinished);
  connect(m_page, &QWebEnginePage::titleChanged, this, &BrowserTab::onTitleChanged);
  connect(m_page, &QWebEnginePage::renderProcessTerminated, this, &BrowserTab::onRenderProcessTerminated);

  layout()->addWidget(m_view);
  m_pageCreated = true;
}

void BrowserTab::destroyPage() {
  if (!m_pageCreated) {
    return;
  }

  if (m_page) {
    m_page->disconnect();
    m_page->deleteLater();
    m_page = nullptr;
  }

  if (m_view) {
    layout()->removeWidget(m_view);
    m_view->deleteLater();
    m_view = nullptr;
  }

  if (m_profile) {
    m_profile->deref();
    m_profile = nullptr;
  }

  m_pageCreated = false;
}

void BrowserTab::activate() {
  m_freezeTimer->stop();
  m_hibernateTimer->stop();

  const LifecycleState state = m_metadata.lifecycleState();

  if (state == LifecycleState::Frozen) {
    setQtLifecycleState(QWebEnginePage::LifecycleState::Active);
    setLifecycleState(LifecycleState::Active);
    return;
  }

  if (state == LifecycleState::Unloaded || state == LifecycleState::Discarded || !m_page) {
    m_recoveryAttempted = false;
    setLifecycleState(LifecycleState::Loading);
    createPage();
    if (m_page) {
      m_page->load(m_metadata.url());
    } else {
      setLifecycleState(LifecycleState::Unloaded);
    }
  }
}

void BrowserTab::deactivate() {
  m_freezeTimer->stop();
  m_hibernateTimer->stop();

  if (m_metadata.lifecycleState() == LifecycleState::Active) {
    scheduleFreeze();
  }
}

void BrowserTab::freeze() {
  if (m_metadata.lifecycleState() != LifecycleState::Active) {
    return;
  }

  if (!maySuspendTo(QWebEnginePage::LifecycleState::Frozen)) {
    scheduleFreeze();
    return;
  }

  setQtLifecycleState(QWebEnginePage::LifecycleState::Frozen);
  setLifecycleState(LifecycleState::Frozen);
  scheduleHibernate();
}

void BrowserTab::hibernate() {
  const LifecycleState state = m_metadata.lifecycleState();
  if (state != LifecycleState::Active && state != LifecycleState::Frozen) {
    return;
  }

  if (!maySuspendTo(QWebEnginePage::LifecycleState::Discarded)) {
    return;
  }

  destroyPage();
  setLifecycleState(LifecycleState::Discarded);
}

void BrowserTab::closeTab() {
  setLifecycleState(LifecycleState::Closing);
  m_freezeTimer->stop();
  m_hibernateTimer->stop();
  destroyPage();
}

bool BrowserTab::isSuspended() const {
  return m_metadata.lifecycleState() == LifecycleState::Discarded;
}

bool BrowserTab::isSuspendable() const {
  const LifecycleState state = m_metadata.lifecycleState();
  return state == LifecycleState::Active || state == LifecycleState::Frozen;
}

void BrowserTab::scheduleFreeze() {
  LifecycleManager* manager = LifecycleManager::instance();
  if (!manager->automationEnabled()) {
    return;
  }
  m_freezeTimer->start(static_cast<int>(manager->freezeDelay()));
}

void BrowserTab::scheduleHibernate() {
  LifecycleManager* manager = LifecycleManager::instance();
  if (!manager->automationEnabled()) {
    return;
  }
  m_hibernateTimer->start(static_cast<int>(manager->hibernateDelay()));
}

bool BrowserTab::maySuspendTo(QWebEnginePage::LifecycleState target) const {
  if (!m_page || !SettingsManager::instance()->respectRecommendedState()) {
    return true;
  }

  if (target == QWebEnginePage::LifecycleState::Frozen) {
    return m_recommendedState != QWebEnginePage::LifecycleState::Active;
  }

  if (target == QWebEnginePage::LifecycleState::Discarded) {
    return m_recommendedState == QWebEnginePage::LifecycleState::Discarded;
  }

  return true;
}

void BrowserTab::onQtLifecycleStateChanged(QWebEnginePage::LifecycleState qtState) {
  m_qtLifecycleState = qtState;
}

void BrowserTab::onRecommendedStateChanged(QWebEnginePage::LifecycleState recommendedState) {
  m_recommendedState = recommendedState;
}

void BrowserTab::onLoadStarted() {
  setLifecycleState(LifecycleState::Loading);
  emit loadStarted();
}

void BrowserTab::onLoadFinished(bool success) {
  if (success) {
    m_recoveryAttempted = false;
    setLifecycleState(LifecycleState::Active);
    if (m_page && !m_page->title().isEmpty()) {
      m_metadata.setTitle(m_page->title());
      emit titleChanged(m_metadata.title());
    }
  } else {
    setLifecycleState(LifecycleState::Unloaded);
  }
  emit loadFinished(success);
}

void BrowserTab::onTitleChanged(const QString& title) {
  m_metadata.setTitle(title);
  emit titleChanged(title);
}

void BrowserTab::onRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, int exitCode) {
  emit renderProcessTerminated(static_cast<int>(status), exitCode);

  const LifecycleState state = m_metadata.lifecycleState();
  if (state != LifecycleState::Active && state != LifecycleState::Loading) {
    return;
  }

  destroyPage();
  setLifecycleState(LifecycleState::Loading);

  if (m_recoveryAttempted) {
    setLifecycleState(LifecycleState::Unloaded);
    return;
  }

  m_recoveryAttempted = true;
  activate();
}

void BrowserTab::setQtLifecycleState(QWebEnginePage::LifecycleState state) {
  if (m_page) {
    m_page->setLifecycleState(state);
  }
}

void BrowserTab::setLifecycleState(LifecycleState state) {
  if (!canTransition(m_metadata.lifecycleState(), state)) {
    return;
  }

  m_metadata.setLifecycleState(state);
  emit lifecycleStateChanged(state);
}
