#include "BrowserTab.h"
#include "LifecycleManager.h"
#include "SettingsManager.h"
#include "profiles/ProfileManager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>
#include <QWebEngineProfile>

BrowserTab::BrowserTab(const QString& providerId, const QString& profileId, const QUrl& url,
                       QWidget* parent)
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
  , m_recoveryAttempted(false)
  , m_stack(nullptr)
  , m_viewContainer(nullptr)
  , m_viewLayout(nullptr)
  , m_errorWidget(nullptr)
  , m_errorIconLabel(nullptr)
  , m_errorTitleLabel(nullptr)
  , m_errorMessageLabel(nullptr)
  , m_retryButton(nullptr)
  , m_closeButton(nullptr)
  , m_hasError(false) {
  setupUi();

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

void BrowserTab::setupUi() {
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  m_stack = new QStackedWidget(this);
  layout->addWidget(m_stack);

  m_viewContainer = new QWidget(this);
  m_viewLayout = new QVBoxLayout(m_viewContainer);
  m_viewLayout->setContentsMargins(0, 0, 0, 0);
  m_stack->addWidget(m_viewContainer);

  setupErrorUi();
  m_stack->addWidget(m_errorWidget);

  m_stack->setCurrentWidget(m_viewContainer);
}

void BrowserTab::setupErrorUi() {
  m_errorWidget = new QWidget(this);
  QVBoxLayout* outerLayout = new QVBoxLayout(m_errorWidget);
  outerLayout->setAlignment(Qt::AlignCenter);

  QWidget* card = new QWidget(m_errorWidget);
  card->setObjectName("errorCard");
  card->setStyleSheet(
      "#errorCard {"
      "  background-color: palette(base);"
      "  border: 1px solid palette(mid);"
      "  border-radius: 12px;"
      "  padding: 24px;"
      "}");
  card->setMaximumWidth(480);
  card->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

  QVBoxLayout* cardLayout = new QVBoxLayout(card);
  cardLayout->setSpacing(12);
  cardLayout->setAlignment(Qt::AlignCenter);

  m_errorIconLabel = new QLabel(card);
  m_errorIconLabel->setAlignment(Qt::AlignCenter);
  m_errorIconLabel->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(48, 48));
  cardLayout->addWidget(m_errorIconLabel);

  m_errorTitleLabel = new QLabel(card);
  m_errorTitleLabel->setAlignment(Qt::AlignCenter);
  QFont titleFont = m_errorTitleLabel->font();
  titleFont.setPointSize(titleFont.pointSize() + 3);
  titleFont.setBold(true);
  m_errorTitleLabel->setFont(titleFont);
  cardLayout->addWidget(m_errorTitleLabel);

  m_errorMessageLabel = new QLabel(card);
  m_errorMessageLabel->setAlignment(Qt::AlignCenter);
  m_errorMessageLabel->setWordWrap(true);
  cardLayout->addWidget(m_errorMessageLabel);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing(10);
  buttonLayout->setAlignment(Qt::AlignCenter);

  m_retryButton = new QPushButton("Retry", card);
  m_retryButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
  m_retryButton->setToolTip("Attempt to reload the page");
  connect(m_retryButton, &QPushButton::clicked, this, &BrowserTab::retry);
  buttonLayout->addWidget(m_retryButton);

  m_closeButton = new QPushButton("Close Tab", card);
  m_closeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  m_closeButton->setToolTip("Close this tab");
  connect(m_closeButton, &QPushButton::clicked, this, [this]() {
    emit closeRequested();
    closeTab();
  });
  buttonLayout->addWidget(m_closeButton);

  cardLayout->addLayout(buttonLayout);
  outerLayout->addWidget(card);
}

void BrowserTab::showError(const QString& title, const QString& message) {
  m_hasError = true;
  m_errorTitle = title;
  m_errorMessage = message;
  m_errorTitleLabel->setText(title);
  m_errorMessageLabel->setText(message);
  m_stack->setCurrentWidget(m_errorWidget);
  emit errorChanged(true);
}

void BrowserTab::hideError() {
  m_hasError = false;
  m_errorTitle.clear();
  m_errorMessage.clear();
  m_stack->setCurrentWidget(m_viewContainer);
  emit errorChanged(false);
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

  m_view = new BrowserView(m_viewContainer);
  m_page = new QWebEnginePage(m_profile->webEngineProfile(), m_view);
  m_view->setPage(m_page);

  m_qtLifecycleState = m_page->lifecycleState();
  m_recommendedState = m_page->recommendedState();

  connect(m_page, &QWebEnginePage::lifecycleStateChanged, this,
          &BrowserTab::onQtLifecycleStateChanged);
  connect(m_page, &QWebEnginePage::recommendedStateChanged, this,
          &BrowserTab::onRecommendedStateChanged);
  connect(m_page, &QWebEnginePage::loadStarted, this, &BrowserTab::onLoadStarted);
  connect(m_page, &QWebEnginePage::loadFinished, this, &BrowserTab::onLoadFinished);
  connect(m_page, &QWebEnginePage::titleChanged, this, &BrowserTab::onTitleChanged);
  connect(m_page, &QWebEnginePage::renderProcessTerminated, this,
          &BrowserTab::onRenderProcessTerminated);

  m_viewLayout->addWidget(m_view);
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
    m_viewLayout->removeWidget(m_view);
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

  if (m_hasError) {
    return;
  }

  const LifecycleState state = m_metadata.lifecycleState();

  if (state == LifecycleState::Frozen) {
    setQtLifecycleState(QWebEnginePage::LifecycleState::Active);
    setLifecycleState(LifecycleState::Active);
    return;
  }

  if (state == LifecycleState::Unloaded || state == LifecycleState::Discarded || !m_page) {
    setLifecycleState(LifecycleState::Loading);
    createPage();
    if (m_page) {
      m_page->load(m_metadata.url());
    } else {
      setLifecycleState(LifecycleState::Unloaded);
      showError("Error Creating Page", "Could not create browser page resources.");
    }
  }
}

void BrowserTab::retry() {
  hideError();
  m_recoveryAttempted = false;
  destroyPage();
  activate();
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
  hideError();
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
  if (m_hasError) {
    hideError();
  }
  setLifecycleState(LifecycleState::Loading);
  emit loadStarted();
}

void BrowserTab::onLoadFinished(bool success) {
  if (success) {
    m_recoveryAttempted = false;
    hideError();
    setLifecycleState(LifecycleState::Active);
    if (m_page && !m_page->title().isEmpty()) {
      m_metadata.setTitle(m_page->title());
      emit titleChanged(m_metadata.title());
    }
  } else {
    setLifecycleState(LifecycleState::Unloaded);
    showError(
        "Unable to Load Page",
        QString("Could not load \"%1\". Check your network connection or verify provider availability.")
            .arg(m_metadata.url().toString()));
  }
  emit loadFinished(success);
}

void BrowserTab::onTitleChanged(const QString& title) {
  m_metadata.setTitle(title);
  emit titleChanged(title);
}

void BrowserTab::onRenderProcessTerminated(
    QWebEnginePage::RenderProcessTerminationStatus status, int exitCode) {
  emit renderProcessTerminated(static_cast<int>(status), exitCode);

  const LifecycleState state = m_metadata.lifecycleState();
  if (state != LifecycleState::Active && state != LifecycleState::Loading) {
    return;
  }

  destroyPage();
  setLifecycleState(LifecycleState::Loading);

  if (m_recoveryAttempted) {
    setLifecycleState(LifecycleState::Unloaded);
    QString reasonText;
    switch (status) {
      case QWebEnginePage::CrashedTerminationStatus:
        reasonText = "The web process crashed unexpectedly.";
        break;
      case QWebEnginePage::KilledTerminationStatus:
        reasonText = "The web process was terminated by the system (out of memory).";
        break;
      case QWebEnginePage::AbnormalTerminationStatus:
        reasonText = "The web process terminated abnormally.";
        break;
      default:
        reasonText = "The web process terminated unexpectedly.";
        break;
    }
    showError(
        "Web Page Unresponsive",
        QString("%1 (Exit code: %2). Automatic recovery failed. You can retry loading or close the tab.")
            .arg(reasonText, QString::number(exitCode)));
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
