#include "BrowserTab.h"
#include "LifecycleManager.h"
#include "SettingsManager.h"
#include "profiles/ProfileManager.h"
#include "providers/Provider.h"
#include "providers/ProviderRegistry.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>
#include <QWebEngineCertificateError>
#include <QWebEngineProfile>

namespace {
constexpr int kLoadTimeoutMs = 45000;
}

BrowserTab::BrowserTab(const QString& providerId, const QString& profileId, const QUrl& url,
                       LifecycleState initialState, QWidget* parent)
  : QWidget(parent)
  , m_metadata(providerId, profileId, url)
  , m_profile(nullptr)
  , m_view(nullptr)
  , m_page(nullptr)
  , m_freezeTimer(nullptr)
  , m_hibernateTimer(nullptr)
  , m_loadTimer(nullptr)
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
  m_metadata.setLifecycleState(initialState);
  setupUi();

  m_freezeTimer = new QTimer(this);
  m_freezeTimer->setSingleShot(true);
  connect(m_freezeTimer, &QTimer::timeout, this, &BrowserTab::freeze);

  m_hibernateTimer = new QTimer(this);
  m_hibernateTimer->setSingleShot(true);
  connect(m_hibernateTimer, &QTimer::timeout, this, &BrowserTab::hibernate);

  m_loadTimer = new QTimer(this);
  m_loadTimer->setSingleShot(true);
  m_loadTimer->setInterval(kLoadTimeoutMs);
  connect(m_loadTimer, &QTimer::timeout, this, &BrowserTab::onLoadTimeout);
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
  const bool isDark = (SettingsManager::instance()->theme() == "dark");
  m_viewContainer->setStyleSheet(isDark ? "background-color: #161616;" : "background-color: #f8f9fa;");
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
    qWarning() << "[WebEngine] Failed to get/create profile for provider:" << m_metadata.providerId();
    return;
  }

  m_profile->ref();

  m_view = new BrowserView(m_viewContainer);
  m_page = new QWebEnginePage(m_profile->webEngineProfile(), m_view);
  m_view->setPage(m_page);

  // Set application-owned background natively without invasive JS injection
  const bool isDark = (SettingsManager::instance()->theme() == "dark");
  const QColor bg = isDark ? QColor("#161616") : QColor("#FFFFFF");
  m_page->setBackgroundColor(bg);

  QPalette pal = m_view->palette();
  pal.setColor(QPalette::Window, bg);
  pal.setColor(QPalette::Base, bg);
  m_view->setPalette(pal);
  m_view->setAutoFillBackground(true);

  m_qtLifecycleState = m_page->lifecycleState();
  m_recommendedState = m_page->recommendedState();

  connect(m_page, &QWebEnginePage::lifecycleStateChanged, this,
          &BrowserTab::onQtLifecycleStateChanged);
  connect(m_page, &QWebEnginePage::recommendedStateChanged, this,
          &BrowserTab::onRecommendedStateChanged);
  connect(m_page, &QWebEnginePage::loadStarted, this, &BrowserTab::onLoadStarted);
  connect(m_page, &QWebEnginePage::loadProgress, this, &BrowserTab::onLoadProgress);
  connect(m_page, &QWebEnginePage::loadFinished, this, &BrowserTab::onLoadFinished);
  connect(m_page, &QWebEnginePage::titleChanged, this, &BrowserTab::onTitleChanged);
  connect(m_page, &QWebEnginePage::urlChanged, this, &BrowserTab::onUrlChanged);
  connect(m_page, &QWebEnginePage::renderProcessTerminated, this,
          &BrowserTab::onRenderProcessTerminated);
  connect(m_page, &QWebEnginePage::certificateError, this,
          [this](const QWebEngineCertificateError& error) {
            qWarning() << "[WebEngine] Certificate error for provider:" << m_metadata.providerId()
                       << "URL:" << error.url().toString()
                       << "Description:" << error.description()
                       << "Overridable:" << error.isOverridable();
          });

  m_viewLayout->addWidget(m_view);
  m_pageCreated = true;

  qInfo() << "[WebEngine] Created page for provider:" << m_metadata.providerId()
          << "URL:" << m_metadata.url().toString()
          << "Dark theme:" << isDark;
}

void BrowserTab::destroyPage() {
  if (!m_pageCreated) {
    return;
  }

  qInfo() << "[WebEngine] Destroying page for provider:" << m_metadata.providerId();

  if (m_loadTimer) {
    m_loadTimer->stop();
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

void BrowserTab::updateTheme(const QString& theme) {
  const bool isDark = (theme == "dark");
  const QColor bg = isDark ? QColor("#161616") : QColor("#FFFFFF");
  if (m_viewContainer) {
    m_viewContainer->setStyleSheet(isDark ? "background-color: #161616;" : "background-color: #f8f9fa;");
  }
  if (m_view) {
    QPalette pal = m_view->palette();
    pal.setColor(QPalette::Window, bg);
    pal.setColor(QPalette::Base, bg);
    m_view->setPalette(pal);
  }
  if (m_page) {
    m_page->setBackgroundColor(bg);
  }
}

void BrowserTab::activate() {
  m_freezeTimer->stop();
  m_hibernateTimer->stop();

  if (m_hasError) {
    return;
  }

  const LifecycleState state = m_metadata.lifecycleState();
  qInfo() << "[WebEngine] Activating tab:" << m_metadata.providerId()
          << "current state:" << lifecycleStateToString(state);

  if (state == LifecycleState::Frozen && m_page) {
    setQtLifecycleState(QWebEnginePage::LifecycleState::Active);
    setLifecycleState(LifecycleState::Active);
    return;
  }

  if (state == LifecycleState::Unloaded || state == LifecycleState::Discarded || !m_page) {
    if (m_metadata.lifecycleState() != LifecycleState::Loading) {
      setLifecycleState(LifecycleState::Loading);
    }
    createPage();
    if (m_page) {
      qInfo() << "[WebEngine] Loading URL:" << m_metadata.url().toString();
      m_loadTimer->start();
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
    return m_recommendedState != QWebEnginePage::LifecycleState::Active;
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
  qInfo() << "[WebEngine] Navigation started for provider:" << m_metadata.providerId()
          << "URL:" << m_metadata.url().toString();
  if (m_hasError) {
    hideError();
  }
  if (m_metadata.lifecycleState() != LifecycleState::Loading) {
    setLifecycleState(LifecycleState::Loading);
  }
  if (m_loadTimer) {
    m_loadTimer->start();
  }
  emit loadStarted();
}

void BrowserTab::onLoadProgress(int progress) {
  if (progress == 0 || progress == 100 || progress % 25 == 0) {
    qInfo() << "[WebEngine] Load progress for provider:" << m_metadata.providerId()
            << "Progress:" << progress;
  }
}

void BrowserTab::onLoadTimeout() {
  if (m_metadata.lifecycleState() != LifecycleState::Loading) {
    return;
  }
  qWarning() << "[WebEngine] Navigation timed out for provider:" << m_metadata.providerId()
             << "URL:" << m_metadata.url().toString();
  if (m_page) {
    m_page->triggerAction(QWebEnginePage::Stop);
  }
  setLifecycleState(LifecycleState::Unloaded);
  showError("Load Timed Out",
            QString("Loading \"%1\" did not complete in time. Check your network connection "
                    "and try again.")
                .arg(m_metadata.url().toString()));
}

void BrowserTab::onLoadFinished(bool success) {
  if (m_loadTimer) {
    m_loadTimer->stop();
  }
  qInfo() << "[WebEngine] Navigation finished for provider:" << m_metadata.providerId()
          << "URL:" << m_metadata.url().toString()
          << "Success:" << success
          << "Current state:" << lifecycleStateToString(m_metadata.lifecycleState());
  if (success) {
    m_recoveryAttempted = false;
    hideError();
    if (m_metadata.lifecycleState() != LifecycleState::Active) {
      setLifecycleState(LifecycleState::Active);
    }
    if (m_page) {
      onUrlChanged(m_page->url());
    }
    if (m_page && !m_page->title().isEmpty()) {
      onTitleChanged(m_page->title());
    }
  } else {
    if (m_metadata.lifecycleState() != LifecycleState::Unloaded) {
      setLifecycleState(LifecycleState::Unloaded);
    }
    showError(
        "Unable to Load Page",
        QString("Could not load \"%1\". Check your network connection or verify provider availability.")
            .arg(m_metadata.url().toString()));
  }
  emit loadFinished(success);
}

void BrowserTab::onTitleChanged(const QString& title) {
  QString cleanTitle = title.trimmed();
  // Filter out URLs, hostnames, and empty strings to maintain consistent provider display names
  if (cleanTitle.isEmpty() || cleanTitle.startsWith("http://") || cleanTitle.startsWith("https://")
      || cleanTitle == m_metadata.url().host() || cleanTitle == m_metadata.url().toString()) {
    return;
  }
  qInfo() << "[WebEngine] Page title updated for provider:" << m_metadata.providerId()
          << "Title:" << cleanTitle;
  m_metadata.setTitle(cleanTitle);
  emit titleChanged(cleanTitle);
}

void BrowserTab::onUrlChanged(const QUrl& url) {
  if (m_metadata.lifecycleState() != LifecycleState::Active) {
    return;
  }

  Provider* provider = ProviderRegistry::instance()->providerById(m_metadata.providerId());
  if (!provider || !provider->isUrlAllowed(url) || url == m_metadata.url()) {
    return;
  }

  qInfo() << "[WebEngine] URL updated for provider:" << m_metadata.providerId()
          << "URL:" << url.toString();
  m_metadata.setUrl(url);
  emit urlChanged(url);
}

void BrowserTab::onRenderProcessTerminated(
    QWebEnginePage::RenderProcessTerminationStatus status, int exitCode) {
  qWarning() << "[WebEngine] Render process terminated for provider:" << m_metadata.providerId()
             << "Status:" << static_cast<int>(status) << "Exit code:" << exitCode;
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
    qWarning() << "[WebEngine] Invalid state transition from"
               << lifecycleStateToString(m_metadata.lifecycleState())
               << "to" << lifecycleStateToString(state)
               << "for provider:" << m_metadata.providerId();
    return;
  }

  qInfo() << "[WebEngine] Lifecycle transition:"
          << lifecycleStateToString(m_metadata.lifecycleState()) << "->"
          << lifecycleStateToString(state)
          << "for provider:" << m_metadata.providerId();

  m_metadata.setLifecycleState(state);
  emit lifecycleStateChanged(state);
}
