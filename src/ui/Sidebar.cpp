#include "Sidebar.h"

#include "LifecycleManager.h"
#include "Provider.h"
#include "ProviderIcons.h"
#include "SettingsManager.h"
#include "TabStore.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/TabMetadata.h"
#include "providers/ProviderRegistry.h"
#include "ui/ToolbarIcons.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>

namespace {

constexpr int kDefaultSidebarWidth = 260;
constexpr int kCollapsedSidebarWidth = 60;
constexpr int kTabRowHeight = 48;
constexpr int kProviderButtonSize = 42;
constexpr int kProviderStripSpacing = 5;
constexpr int kProviderStripMargin = 2;
constexpr int kProviderStripHeight = kProviderButtonSize + 2 * kProviderStripMargin;

}  // namespace

class TabRowWidget : public QWidget {
  Q_OBJECT
public:
  explicit TabRowWidget(const QString& tabId, QWidget* parent = nullptr)
    : QWidget(parent), m_tabId(tabId) {
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_StyledBackground, true);
  }

  QString tabId() const { return m_tabId; }

protected:
  bool event(QEvent* event) override {
    if (event->type() == QEvent::Enter) {
      emit entered();
    } else if (event->type() == QEvent::Leave) {
      emit left();
    }
    return QWidget::event(event);
  }

signals:
  void entered();
  void left();

private:
  QString m_tabId;
};

class ProviderScrollArea : public QScrollArea {
  Q_OBJECT
public:
  explicit ProviderScrollArea(QWidget* parent = nullptr)
    : QScrollArea(parent) {
  }

protected:
  void wheelEvent(QWheelEvent* event) override {
    QScrollBar* bar = horizontalScrollBar();
    if (!bar || bar->maximum() == 0) {
      QScrollArea::wheelEvent(event);
      return;
    }

    int delta = event->angleDelta().y();
    if (delta == 0) {
      delta = event->angleDelta().x();
    }
    if (delta == 0) {
      delta = event->pixelDelta().y() + event->pixelDelta().x();
    }
    if (delta == 0) {
      QScrollArea::wheelEvent(event);
      return;
    }

    bar->setValue(bar->value() - delta);
    event->accept();
  }
};

Sidebar::Sidebar(QWidget* parent)
  : QWidget(parent)
  , m_expandedContainer(nullptr)
  , m_collapsedRail(nullptr)
  , m_headerAppIcon(nullptr)
  , m_headerAppLabel(nullptr)
  , m_collapsedTabsButton(nullptr)
  , m_providerScrollArea(nullptr)
  , m_providerStrip(nullptr)
  , m_tabList(nullptr)
  , m_collapseButton(nullptr)
  , m_settingsButton(nullptr)
  , m_sidebarWidth(kDefaultSidebarWidth)
  , m_collapsed(false) {
  setObjectName("sidebar");
  setAttribute(Qt::WA_StyledBackground, true);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  setMinimumWidth(kCollapsedSidebarWidth);
  setMaximumWidth(kDefaultSidebarWidth);

  setupUi();
}

void Sidebar::setupUi() {
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Header
  QWidget* header = new QWidget(this);
  header->setObjectName("sidebarHeader");
  header->setFixedHeight(56);
  QHBoxLayout* headerLayout = new QHBoxLayout(header);
  headerLayout->setContentsMargins(14, 0, 14, 0);
  headerLayout->setSpacing(10);

  m_headerAppIcon = new QLabel(header);
  m_headerAppIcon->setObjectName("sidebarAppIcon");
  m_headerAppIcon->setFixedSize(28, 28);
  m_headerAppIcon->setScaledContents(true);
  QPixmap appPixmap(":/icons/hicolor/256x256/apps/ai-workspace.png");
  if (appPixmap.isNull()) {
    appPixmap = QPixmap(32, 32);
    appPixmap.fill(Qt::transparent);
    QPainter p(&appPixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(QColor("#3584e4"));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(0, 0, 32, 32, 8, 8);
    p.setPen(Qt::white);
    QFont f = p.font();
    f.setBold(true);
    f.setPixelSize(16);
    p.setFont(f);
    p.drawText(QRectF(0, 0, 32, 32), Qt::AlignCenter, "AI");
  }
  m_headerAppIcon->setPixmap(appPixmap);
  m_headerAppIcon->setToolTip("AI Workspace");
  headerLayout->addWidget(m_headerAppIcon);

  m_headerAppLabel = new QLabel("AI Workspace", header);
  QFont appFont = m_headerAppLabel->font();
  appFont.setPointSize(15);
  appFont.setBold(true);
  m_headerAppLabel->setFont(appFont);
  m_headerAppLabel->setObjectName("sidebarAppTitle");
  headerLayout->addWidget(m_headerAppLabel);

  mainLayout->addWidget(header);

  // Divider
  QFrame* divider = new QFrame(this);
  divider->setFrameShape(QFrame::HLine);
  divider->setFixedHeight(1);
  divider->setObjectName("sidebarDivider");
  mainLayout->addWidget(divider);

  // Expanded container
  m_expandedContainer = new QWidget(this);
  m_expandedContainer->setObjectName("sidebarExpandedContainer");
  QVBoxLayout* expandedLayout = new QVBoxLayout(m_expandedContainer);
  expandedLayout->setContentsMargins(0, 0, 0, 0);
  expandedLayout->setSpacing(0);

  setupProviderSection();
  setupTabsSection();
  mainLayout->addWidget(m_expandedContainer);

  // Collapsed rail
  setupCollapsedRail();
  mainLayout->addWidget(m_collapsedRail);
  m_collapsedRail->setVisible(false);

  // Bottom section
  setupBottomSection();
}

void Sidebar::setupProviderSection() {
  QWidget* section = new QWidget(m_expandedContainer ? m_expandedContainer : this);
  section->setObjectName("providersSection");
  QVBoxLayout* sectionLayout = new QVBoxLayout(section);
  sectionLayout->setContentsMargins(12, 8, 12, 8);
  sectionLayout->setSpacing(6);

  QLabel* sectionLabel = new QLabel("PROVIDERS", section);
  QFont labelFont = sectionLabel->font();
  labelFont.setPointSize(10);
  labelFont.setBold(true);
  labelFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
  sectionLabel->setFont(labelFont);
  sectionLabel->setObjectName("sidebarSectionLabel");
  sectionLayout->addWidget(sectionLabel);

  m_providerScrollArea = new ProviderScrollArea(section);
  m_providerScrollArea->setObjectName("providerScrollArea");
  m_providerScrollArea->setFrameShape(QFrame::NoFrame);
  m_providerScrollArea->setWidgetResizable(false);
  m_providerScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_providerScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_providerScrollArea->setFocusPolicy(Qt::NoFocus);
  m_providerScrollArea->setFixedHeight(kProviderStripHeight + 4);
  m_providerScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_providerScrollArea->viewport()->setAutoFillBackground(false);

  m_providerStrip = new QWidget(m_providerScrollArea);
  m_providerStrip->setObjectName("providerStrip");
  m_providerStrip->setAutoFillBackground(false);
  m_providerStrip->setFixedHeight(kProviderStripHeight);
  QHBoxLayout* stripLayout = new QHBoxLayout(m_providerStrip);
  stripLayout->setContentsMargins(kProviderStripMargin, kProviderStripMargin,
                                  kProviderStripMargin, kProviderStripMargin);
  stripLayout->setSpacing(kProviderStripSpacing);
  stripLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_providerScrollArea->setWidget(m_providerStrip);
  sectionLayout->addWidget(m_providerScrollArea);

  ProviderRegistry* registry = ProviderRegistry::instance();
  for (Provider* provider : registry->providers()) {
    addProviderRow(provider->id(), provider->displayName(), provider->icon());
  }

  const int count = registry->providers().size();
  const int stripWidth = 2 * kProviderStripMargin + count * kProviderButtonSize
                         + qMax(0, count - 1) * kProviderStripSpacing;
  m_providerStrip->setFixedWidth(stripWidth);

  if (m_expandedContainer && m_expandedContainer->layout()) {
    static_cast<QVBoxLayout*>(m_expandedContainer->layout())->addWidget(section, 0);
  } else {
    layout()->addWidget(section);
  }
}

void Sidebar::setupTabsSection() {
  QWidget* section = new QWidget(m_expandedContainer ? m_expandedContainer : this);
  section->setObjectName("tabsSection");
  QVBoxLayout* sectionLayout = new QVBoxLayout(section);
  sectionLayout->setContentsMargins(12, 8, 12, 4);
  sectionLayout->setSpacing(4);

  QLabel* sectionLabel = new QLabel("OPEN TABS", section);
  QFont labelFont = sectionLabel->font();
  labelFont.setPointSize(10);
  labelFont.setBold(true);
  labelFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
  sectionLabel->setFont(labelFont);
  sectionLabel->setObjectName("sidebarSectionLabel");
  sectionLayout->addWidget(sectionLabel);

  m_tabList = new QListWidget(this);
  m_tabList->setObjectName("tabList");
  m_tabList->setSelectionMode(QListWidget::SingleSelection);
  m_tabList->setFrameShape(QListWidget::NoFrame);
  m_tabList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_tabList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_tabList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_tabList->setAlternatingRowColors(false);

  connect(m_tabList, &QListWidget::itemClicked, this, &Sidebar::onItemClicked);
  connect(m_tabList, &QListWidget::customContextMenuRequested, this,
          &Sidebar::showTabContextMenu);

  sectionLayout->addWidget(m_tabList);

  if (m_expandedContainer && m_expandedContainer->layout()) {
    static_cast<QVBoxLayout*>(m_expandedContainer->layout())->addWidget(section, 1);
  } else {
    layout()->addWidget(section);
  }
}

void Sidebar::setupCollapsedRail() {
  m_collapsedRail = new QWidget(this);
  m_collapsedRail->setObjectName("collapsedRail");
  QVBoxLayout* railLayout = new QVBoxLayout(m_collapsedRail);
  railLayout->setContentsMargins(6, 6, 6, 6);
  railLayout->setSpacing(4);
  railLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

  ProviderRegistry* registry = ProviderRegistry::instance();
  for (Provider* provider : registry->providers()) {
    QToolButton* btn = new QToolButton(m_collapsedRail);
    btn->setObjectName("collapsedProviderButton");
    btn->setFixedSize(36, 36);
    btn->setIcon(provider->icon());
    btn->setIconSize(QSize(26, 26));
    btn->setToolTip(provider->displayName());
    btn->setProperty("providerId", provider->id());
    btn->setProperty("selected", false);
    btn->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(btn, &QToolButton::clicked, this, [this, provider]() {
      emit providerSelected(provider->id());
    });
    connect(btn, &QToolButton::customContextMenuRequested, this,
            [this, provider, btn](const QPoint& pos) {
              QMenu menu(this);
              menu.addAction("Add Account", this, [this, provider]() {
                emit providerAddAccountRequested(provider->id());
              });
              menu.addAction("Manage Accounts", this, [this, provider]() {
                emit providerManageAccountsRequested(provider->id());
              });
              menu.exec(btn->mapToGlobal(pos));
            });
    m_collapsedProviderButtons.insert(provider->id(), btn);
    railLayout->addWidget(btn);
  }

  QFrame* railDivider = new QFrame(m_collapsedRail);
  railDivider->setFrameShape(QFrame::HLine);
  railDivider->setFixedHeight(1);
  railDivider->setObjectName("sidebarDivider");
  railLayout->addWidget(railDivider);

  const bool isDark = (SettingsManager::instance()->theme() == "dark");
  m_collapsedTabsButton = new QToolButton(m_collapsedRail);
  m_collapsedTabsButton->setObjectName("collapsedTabsButton");
  m_collapsedTabsButton->setFixedSize(36, 36);
  m_collapsedTabsButton->setIcon(ToolbarIcons::tabsIcon(isDark));
  m_collapsedTabsButton->setIconSize(QSize(20, 20));
  m_collapsedTabsButton->setToolTip("Open Tabs (0)");
  connect(m_collapsedTabsButton, &QToolButton::clicked, this, &Sidebar::showCollapsedTabsMenu);
  railLayout->addWidget(m_collapsedTabsButton);

  railLayout->addStretch();
}

void Sidebar::showCollapsedTabsMenu() {
  QMenu menu(this);
  if (m_tabItems.isEmpty()) {
    QAction* empty = menu.addAction("No tabs open");
    empty->setEnabled(false);
  } else {
    for (auto it = m_tabItems.begin(); it != m_tabItems.end(); ++it) {
      QString tabId = it.key();
      QListWidgetItem* item = it.value();
      QString tooltip = item ? item->toolTip() : tabId;
      QString providerId = m_tabIdToProviderId.value(tabId);
      Provider* p = ProviderRegistry::instance()->providerById(providerId);
      QIcon icon = p ? p->icon() : QIcon();
      QAction* act = menu.addAction(icon, tooltip);
      connect(act, &QAction::triggered, this, [this, tabId]() {
        emit tabSelected(tabId);
      });
    }
  }
  menu.exec(m_collapsedTabsButton->mapToGlobal(QPoint(m_collapsedTabsButton->width() + 4, 0)));
}

void Sidebar::updateCollapsedTabsButton() {
  if (m_collapsedTabsButton) {
    m_collapsedTabsButton->setToolTip(QString("Open Tabs (%1)").arg(m_tabItems.size()));
  }
}

void Sidebar::setSelectedProvider(const QString& providerId) {
  m_selectedProviderId = providerId;

  for (auto it = m_providerButtons.begin(); it != m_providerButtons.end(); ++it) {
    applySelectedProvider(it.value(), it.key());
  }
  for (auto it = m_collapsedProviderButtons.begin(); it != m_collapsedProviderButtons.end(); ++it) {
    applySelectedProvider(it.value(), it.key());
  }

  QToolButton* selected = m_providerButtons.value(providerId, nullptr);
  if (selected && m_providerScrollArea) {
    m_providerScrollArea->ensureWidgetVisible(selected, 8, 0);
  }
}

void Sidebar::applySelectedProvider(QToolButton* button, const QString& providerId) {
  if (!button) {
    return;
  }

  const bool selected = !m_selectedProviderId.isEmpty() && providerId == m_selectedProviderId;
  if (button->property("selected").toBool() != selected) {
    button->setProperty("selected", selected);
    button->style()->unpolish(button);
    button->style()->polish(button);
    button->update();
  }
}

void Sidebar::setupBottomSection() {
  QWidget* bottomSection = new QWidget(this);
  bottomSection->setObjectName("sidebarBottomSection");
  QHBoxLayout* bottomLayout = new QHBoxLayout(bottomSection);
  bottomLayout->setContentsMargins(12, 8, 12, 8);
  bottomLayout->setSpacing(8);

  const bool isDark = (SettingsManager::instance()->theme() == "dark");

  m_collapseButton = new QToolButton(this);
  m_collapseButton->setObjectName("sidebarCollapseButton");
  m_collapseButton->setToolTip("Collapse sidebar");
  m_collapseButton->setIcon(ToolbarIcons::backIcon(isDark));
  m_collapseButton->setIconSize(QSize(16, 16));
  m_collapseButton->setFixedSize(28, 28);
  m_collapseButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  connect(m_collapseButton, &QToolButton::clicked, this, [this]() {
    setCollapsed(!m_collapsed);
    emit sidebarCollapsedChanged(m_collapsed);
  });
  bottomLayout->addWidget(m_collapseButton);

  bottomLayout->addStretch();

  m_settingsButton = new QToolButton(this);
  m_settingsButton->setObjectName("sidebarSettingsButton");
  m_settingsButton->setToolTip("Settings");
  m_settingsButton->setIcon(ToolbarIcons::settingsIcon(isDark));
  m_settingsButton->setIconSize(QSize(16, 16));
  m_settingsButton->setFixedSize(28, 28);
  m_settingsButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  connect(m_settingsButton, &QToolButton::clicked, this, &Sidebar::settingsRequested);
  bottomLayout->addWidget(m_settingsButton);

  layout()->addWidget(bottomSection);
}

void Sidebar::addProviderRow(const QString& providerId, const QString& displayName,
                             const QIcon& icon) {
  if (!m_providerStrip) {
    return;
  }

  QToolButton* button = new QToolButton(m_providerStrip);
  button->setObjectName("providerStripButton");
  button->setFixedSize(kProviderButtonSize, kProviderButtonSize);
  button->setIcon(icon);
  button->setIconSize(QSize(28, 28));
  button->setToolTip(displayName);
  button->setToolButtonStyle(Qt::ToolButtonIconOnly);
  button->setProperty("providerId", providerId);
  button->setProperty("selected", false);
  button->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(button, &QToolButton::clicked, this, [this, providerId]() {
    emit providerSelected(providerId);
  });
  connect(button, &QToolButton::customContextMenuRequested, this,
          [this, button](const QPoint& pos) {
            showProviderContextMenu(button, pos);
          });

  m_providerButtons.insert(providerId, button);
  static_cast<QHBoxLayout*>(m_providerStrip->layout())->addWidget(button);
  applySelectedProvider(button, providerId);
}

void Sidebar::addTabRow(const QString& tabId, const QString& providerId,
                        const QString& title, const QIcon& icon, LifecycleState state,
                        bool hasError) {
  if (m_tabItems.contains(tabId)) {
    return;
  }

  QListWidgetItem* item = new QListWidgetItem(m_tabList);
  item->setData(Qt::UserRole, tabId);
  item->setData(Qt::UserRole + 1, "tab");
  item->setSizeHint(QSize(0, kTabRowHeight));

  TabRowWidget* widget = new TabRowWidget(tabId, m_tabList);
  widget->setObjectName("tabRowWidget");
  QHBoxLayout* layout = new QHBoxLayout(widget);
  layout->setContentsMargins(10, 6, 6, 6);
  layout->setSpacing(8);

  QLabel* iconLabel = new QLabel(widget);
  iconLabel->setFixedSize(28, 28);
  iconLabel->setPixmap(icon.pixmap(28, 28));
  iconLabel->setScaledContents(true);
  iconLabel->setObjectName("tabIconLabel");
  layout->addWidget(iconLabel);

  QLabel* titleLabel = new QLabel(title, widget);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(11);
  titleLabel->setFont(titleFont);
  titleLabel->setObjectName("tabTitleLabel");
  titleLabel->setWordWrap(false);
  layout->addWidget(titleLabel);

  layout->addStretch();

  QLabel* stateBadge = new QLabel(widget);
  stateBadge->setObjectName("tabStateBadge");
  stateBadge->setFixedSize(16, 16);
  stateBadge->setAlignment(Qt::AlignCenter);
  QFont badgeFont = stateBadge->font();
  badgeFont.setPointSize(7);
  badgeFont.setBold(true);
  stateBadge->setFont(badgeFont);
  layout->addWidget(stateBadge);

  QToolButton* closeButton = new QToolButton(widget);
  closeButton->setObjectName("tabCloseButton");
  closeButton->setFixedSize(20, 20);
  const bool isDark = (SettingsManager::instance()->theme() == "dark");
  closeButton->setIcon(ToolbarIcons::closeIcon(isDark));
  closeButton->setIconSize(QSize(10, 10));
  closeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  closeButton->setToolTip("Close tab");
  closeButton->setVisible(false);
  closeButton->setProperty("tabId", tabId);
  connect(closeButton, &QToolButton::clicked, this, [this, closeButton]() {
    QString tabId = closeButton->property("tabId").toString();
    emit tabClosed(tabId);
  });
  layout->addWidget(closeButton);

  // Show close button on hover
  connect(widget, &TabRowWidget::entered, this, [closeButton]() {
    closeButton->setVisible(true);
  });
  connect(widget, &TabRowWidget::left, this, [closeButton]() {
    closeButton->setVisible(false);
  });

  m_tabList->setItemWidget(item, widget);
  m_tabItems.insert(tabId, item);
  m_tabIdToProviderId.insert(tabId, providerId);

  updateTabItemWidget(item, title, state, hasError);
  updateCollapsedTabsButton();
}

void Sidebar::updateTabRow(const QString& tabId, const QString& title,
                           LifecycleState state, bool hasError) {
  QListWidgetItem* item = m_tabItems.value(tabId);
  if (!item) {
    return;
  }
  updateTabItemWidget(item, title, state, hasError);
}

void Sidebar::updateTabItemWidget(QListWidgetItem* item, const QString& title,
                                  LifecycleState state, bool hasError) {
  QWidget* widget = m_tabList->itemWidget(item);
  if (!widget) {
    return;
  }

  QLabel* titleLabel = widget->findChild<QLabel*>("tabTitleLabel");
  if (titleLabel && !title.isEmpty()) {
    titleLabel->setText(title);
  }

  QLabel* stateBadge = widget->findChild<QLabel*>("tabStateBadge");
  if (stateBadge) {
    QString badgeText;
    QColor badgeColor;
    if (hasError) {
      badgeText = "!";
      badgeColor = QColor("#E01B24");
    } else {
      switch (state) {
        case LifecycleState::Active:
          badgeText = "A";
          badgeColor = QColor("#26A269");
          break;
        case LifecycleState::Frozen:
          badgeText = "F";
          badgeColor = QColor("#33C7DE");
          break;
        case LifecycleState::Discarded:
          badgeText = "H";
          badgeColor = QColor("#9A9996");
          break;
        case LifecycleState::Loading:
          badgeText = "L";
          badgeColor = QColor("#3584E4");
          break;
        case LifecycleState::Unloaded:
          badgeText = "U";
          badgeColor = QColor("#77767B");
          break;
        case LifecycleState::Closing:
          badgeText = "C";
          badgeColor = QColor("#77767B");
          break;
      }
    }
    stateBadge->setText(badgeText);
    stateBadge->setStyleSheet(
        QString("background-color: %1; color: #FFFFFF; border-radius: 8px; font-weight: bold;")
            .arg(badgeColor.name()));
  }

  QString tooltip = title;
  if (hasError) {
    tooltip += " (Error)";
  } else {
    tooltip += QString(" — %1").arg(lifecycleStateToString(state));
  }
  item->setToolTip(tooltip);
  widget->setToolTip(tooltip);
}

void Sidebar::removeTabRow(const QString& tabId) {
  QListWidgetItem* item = m_tabItems.take(tabId);
  if (!item) {
    return;
  }
  m_tabIdToProviderId.remove(tabId);
  int row = m_tabList->row(item);
  if (row >= 0) {
    m_tabList->takeItem(row);
  }
  delete item;
  updateCollapsedTabsButton();
}

void Sidebar::selectTabRow(const QString& tabId) {
  QListWidgetItem* item = m_tabItems.value(tabId);
  if (!item) {
    return;
  }
  m_tabList->setCurrentItem(item);
  m_tabList->scrollToItem(item);
  for (auto it = m_tabItems.cbegin(); it != m_tabItems.cend(); ++it) {
    QWidget* widget = m_tabList->itemWidget(it.value());
    if (!widget) {
      continue;
    }
    const bool selected = (it.value() == item);
    if (widget->property("selected").toBool() != selected) {
      widget->setProperty("selected", selected);
      widget->style()->unpolish(widget);
      widget->style()->polish(widget);
      widget->update();
    }
  }
}

void Sidebar::clearTabRows() {
  m_tabItems.clear();
  m_tabIdToProviderId.clear();
  m_tabList->clear();
  updateCollapsedTabsButton();
}

void Sidebar::onItemClicked(QListWidgetItem* item) {
  QString type = item->data(Qt::UserRole + 1).toString();
  if (type == "tab") {
    QString tabId = item->data(Qt::UserRole).toString();
    emit tabSelected(tabId);
  }
}

void Sidebar::showTabContextMenu(const QPoint& pos) {
  QListWidgetItem* item = m_tabList->itemAt(pos);
  if (!item) {
    return;
  }
  QString type = item->data(Qt::UserRole + 1).toString();
  if (type != "tab") {
    return;
  }
  QString tabId = item->data(Qt::UserRole).toString();

  QMenu menu(this);
  menu.addAction("Reload", this, [this, tabId]() { executeTabAction(tabId, "reload"); });
  menu.addAction("Freeze", this, [this, tabId]() { executeTabAction(tabId, "freeze"); });
  menu.addAction("Hibernate", this, [this, tabId]() { executeTabAction(tabId, "hibernate"); });
  menu.addSeparator();
  menu.addAction("Close Tab", this, [this, tabId]() { emit tabClosed(tabId); });
  menu.exec(m_tabList->viewport()->mapToGlobal(pos));
}

void Sidebar::showProviderContextMenu(QToolButton* button, const QPoint& pos) {
  if (!button) {
    return;
  }
  const QString providerId = button->property("providerId").toString();
  if (providerId.isEmpty()) {
    return;
  }

  QMenu menu(this);
  menu.addAction("Add Account", this, [this, providerId]() {
    emit providerAddAccountRequested(providerId);
  });
  menu.addAction("Manage Accounts", this, [this, providerId]() {
    emit providerManageAccountsRequested(providerId);
  });
  menu.exec(button->mapToGlobal(pos));
}

void Sidebar::executeTabAction(const QString& tabId, const QString& action) {
  emit tabActionRequested(tabId, action);
}

int Sidebar::sidebarWidth() const {
  return m_sidebarWidth;
}

void Sidebar::setSidebarWidth(int width) {
  m_sidebarWidth = qBound(kCollapsedSidebarWidth, width, 400);
  if (m_collapsed) {
    setMinimumWidth(kCollapsedSidebarWidth);
    setMaximumWidth(kCollapsedSidebarWidth);
  } else {
    setMinimumWidth(m_sidebarWidth);
    setMaximumWidth(m_sidebarWidth);
  }
  emit sidebarWidthChanged(m_sidebarWidth);
}

bool Sidebar::isCollapsed() const {
  return m_collapsed;
}

void Sidebar::setCollapsed(bool collapsed) {
  m_collapsed = collapsed;
  if (collapsed) {
    setMinimumWidth(kCollapsedSidebarWidth);
    setMaximumWidth(kCollapsedSidebarWidth);
  } else {
    setMinimumWidth(kDefaultSidebarWidth);
    setMaximumWidth(kDefaultSidebarWidth);
  }

  const bool isDark = (SettingsManager::instance()->theme() == "dark");
  m_collapseButton->setIcon(collapsed ? ToolbarIcons::forwardIcon(isDark) : ToolbarIcons::backIcon(isDark));
  m_collapseButton->setToolTip(
      collapsed ? tr("Expand sidebar") : tr("Collapse sidebar"));

  if (m_expandedContainer) {
    m_expandedContainer->setVisible(!collapsed);
  }
  if (m_collapsedRail) {
    m_collapsedRail->setVisible(collapsed);
  }
  if (m_headerAppLabel) {
    m_headerAppLabel->setVisible(!collapsed);
  }
  if (m_headerAppIcon) {
    m_headerAppIcon->setFixedSize(collapsed ? QSize(32, 32) : QSize(28, 28));
  }

  updateCollapsedTabsButton();

  emit sidebarCollapsedChanged(collapsed);
  emit sidebarWidthChanged(collapsed ? kCollapsedSidebarWidth : kDefaultSidebarWidth);
}

void Sidebar::updateTheme(const QString& theme) {
  const bool isDark = (theme == "dark");
  if (m_collapsedTabsButton) {
    m_collapsedTabsButton->setIcon(ToolbarIcons::tabsIcon(isDark));
  }
  if (m_collapseButton) {
    m_collapseButton->setIcon(m_collapsed ? ToolbarIcons::forwardIcon(isDark) : ToolbarIcons::backIcon(isDark));
  }
  if (m_settingsButton) {
    m_settingsButton->setIcon(ToolbarIcons::settingsIcon(isDark));
  }
  for (QWidget* widget : m_tabList->findChildren<QWidget*>("tabRowWidget")) {
    QToolButton* closeButton = widget->findChild<QToolButton*>("tabCloseButton");
    if (closeButton) {
      closeButton->setIcon(ToolbarIcons::closeIcon(isDark));
    }
  }
  setSelectedProvider(m_selectedProviderId);
}

#include "Sidebar.moc"
