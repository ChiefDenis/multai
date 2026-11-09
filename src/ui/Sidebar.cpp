#include "Sidebar.h"

#include "LifecycleManager.h"
#include "Provider.h"
#include "ProviderIcons.h"
#include "SettingsManager.h"
#include "TabStore.h"
#include "lifecycle/BrowserTab.h"
#include "lifecycle/TabMetadata.h"
#include "providers/ProviderRegistry.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

constexpr int kDefaultSidebarWidth = 260;
constexpr int kCollapsedSidebarWidth = 60;
constexpr int kTabRowHeight = 48;
constexpr int kProviderRowHeight = 44;

// Create a light-colored version of a standard icon for use on dark backgrounds.
QIcon makeLightIcon(const QIcon& icon, int size = 16) {
  QPixmap source = icon.pixmap(size, size);
  QPixmap result(size, size);
  result.fill(Qt::transparent);
  QPainter painter(&result);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.drawPixmap(0, 0, source);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(result.rect(), Qt::white);
  painter.end();
  return QIcon(result);
}

}  // namespace

class TabRowWidget : public QWidget {
  Q_OBJECT
public:
  explicit TabRowWidget(const QString& tabId, QWidget* parent = nullptr)
    : QWidget(parent), m_tabId(tabId) {
    setAttribute(Qt::WA_Hover, true);
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

Sidebar::Sidebar(QWidget* parent)
  : QWidget(parent)
  , m_providerList(nullptr)
  , m_tabList(nullptr)
  , m_collapseButton(nullptr)
  , m_settingsButton(nullptr)
  , m_sidebarWidth(kDefaultSidebarWidth)
  , m_collapsed(false) {
  setObjectName("sidebar");
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
  headerLayout->setContentsMargins(16, 0, 16, 0);

  QLabel* appLabel = new QLabel("AI Workspace", header);
  QFont appFont = appLabel->font();
  appFont.setPointSize(16);
  appFont.setBold(true);
  appLabel->setFont(appFont);
  headerLayout->addWidget(appLabel);

  mainLayout->addWidget(header);

  // Divider
  QFrame* divider = new QFrame(this);
  divider->setFrameShape(QFrame::HLine);
  divider->setFixedHeight(1);
  divider->setObjectName("sidebarDivider");
  mainLayout->addWidget(divider);

  // Provider section
  setupProviderSection();

  // Tabs section
  setupTabsSection();

  // Stretch to push bottom section down
  mainLayout->addStretch();

  // Bottom section
  setupBottomSection();
}

void Sidebar::setupProviderSection() {
  QWidget* section = new QWidget(this);
  section->setObjectName("providersSection");
  QVBoxLayout* sectionLayout = new QVBoxLayout(section);
  sectionLayout->setContentsMargins(12, 8, 12, 4);
  sectionLayout->setSpacing(4);

  QLabel* sectionLabel = new QLabel("PROVIDERS", section);
  QFont labelFont = sectionLabel->font();
  labelFont.setPointSize(10);
  labelFont.setBold(true);
  labelFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
  sectionLabel->setFont(labelFont);
  sectionLabel->setObjectName("sidebarSectionLabel");
  sectionLayout->addWidget(sectionLabel);

  m_providerList = new QListWidget(this);
  m_providerList->setObjectName("providerList");
  m_providerList->setSelectionMode(QListWidget::NoSelection);
  m_providerList->setFocusPolicy(Qt::NoFocus);
  m_providerList->setFrameShape(QListWidget::NoFrame);
  m_providerList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_providerList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  connect(m_providerList, &QListWidget::itemClicked, this,
          [this](QListWidgetItem* item) {
            QString providerId = item->data(Qt::UserRole).toString();
            if (!providerId.isEmpty()) {
              emit providerSelected(providerId);
            }
          });

  m_providerList->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_providerList, &QListWidget::customContextMenuRequested, this,
          &Sidebar::showProviderContextMenu);

  sectionLayout->addWidget(m_providerList);

  // Add all providers
  ProviderRegistry* registry = ProviderRegistry::instance();
  for (Provider* provider : registry->providers()) {
    addProviderRow(provider->id(), provider->displayName(), provider->icon());
  }

  layout()->addWidget(section);
}

void Sidebar::setupTabsSection() {
  QWidget* section = new QWidget(this);
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

  layout()->addWidget(section);
}

void Sidebar::setupBottomSection() {
  QWidget* bottomSection = new QWidget(this);
  bottomSection->setObjectName("sidebarBottomSection");
  QHBoxLayout* bottomLayout = new QHBoxLayout(bottomSection);
  bottomLayout->setContentsMargins(12, 8, 12, 8);
  bottomLayout->setSpacing(8);

  m_collapseButton = new QToolButton(this);
  m_collapseButton->setObjectName("sidebarCollapseButton");
  m_collapseButton->setToolTip("Collapse sidebar");
  m_collapseButton->setIcon(makeLightIcon(style()->standardIcon(QStyle::SP_ArrowLeft)));
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
  m_settingsButton->setIcon(makeLightIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView)));
  m_settingsButton->setIconSize(QSize(16, 16));
  m_settingsButton->setFixedSize(28, 28);
  m_settingsButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  connect(m_settingsButton, &QToolButton::clicked, this, &Sidebar::settingsRequested);
  bottomLayout->addWidget(m_settingsButton);

  layout()->addWidget(bottomSection);
}

void Sidebar::addProviderRow(const QString& providerId, const QString& displayName,
                             const QIcon& icon) {
  QListWidgetItem* item = new QListWidgetItem(m_providerList);
  item->setData(Qt::UserRole, providerId);
  item->setData(Qt::UserRole + 1, "provider");
  item->setSizeHint(QSize(0, kProviderRowHeight));

  QWidget* widget = new QWidget(m_providerList);
  QHBoxLayout* layout = new QHBoxLayout(widget);
  layout->setContentsMargins(12, 4, 12, 4);
  layout->setSpacing(10);

  QLabel* iconLabel = new QLabel(widget);
  iconLabel->setFixedSize(24, 24);
  iconLabel->setPixmap(icon.pixmap(24, 24));
  iconLabel->setScaledContents(true);
  layout->addWidget(iconLabel);

  QLabel* nameLabel = new QLabel(displayName, widget);
  QFont nameFont = nameLabel->font();
  nameFont.setPointSize(10);
  nameLabel->setFont(nameFont);
  nameLabel->setObjectName("providerNameLabel");
  nameLabel->setWordWrap(true);
  nameLabel->setMinimumWidth(70);
  nameLabel->setMaximumWidth(170);
  layout->addWidget(nameLabel);

  layout->addStretch();

  m_providerList->setItemWidget(item, widget);
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
  closeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  closeButton->setIconSize(QSize(10, 10));
  closeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
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
}

void Sidebar::selectTabRow(const QString& tabId) {
  QListWidgetItem* item = m_tabItems.value(tabId);
  if (!item) {
    return;
  }
  m_tabList->setCurrentItem(item);
  m_tabList->scrollToItem(item);
}

void Sidebar::clearTabRows() {
  m_tabItems.clear();
  m_tabIdToProviderId.clear();
  m_tabList->clear();
}

void Sidebar::onItemClicked(QListWidgetItem* item) {
  QString type = item->data(Qt::UserRole + 1).toString();
  if (type == "tab") {
    QString tabId = item->data(Qt::UserRole).toString();
    emit tabSelected(tabId);
  } else if (type == "provider") {
    QString providerId = item->data(Qt::UserRole).toString();
    emit providerSelected(providerId);
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

void Sidebar::showProviderContextMenu(const QPoint& pos) {
  QListWidgetItem* item = m_providerList->itemAt(pos);
  if (!item) {
    return;
  }
  QString type = item->data(Qt::UserRole + 1).toString();
  if (type != "provider") {
    return;
  }
  QString providerId = item->data(Qt::UserRole).toString();

  QMenu menu(this);
  menu.addAction("Add Account", this, [this, providerId]() {
    emit providerAddAccountRequested(providerId);
  });
  menu.addAction("Manage Accounts", this, [this, providerId]() {
    emit providerManageAccountsRequested(providerId);
  });
  menu.exec(m_providerList->viewport()->mapToGlobal(pos));
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
  m_collapseButton->setIcon(
      style()->standardIcon(collapsed ? QStyle::SP_ArrowRight : QStyle::SP_ArrowLeft));
  m_collapseButton->setToolTip(
      collapsed ? tr("Expand sidebar") : tr("Collapse sidebar"));

  m_providerList->setVisible(!collapsed);
  m_tabList->setVisible(!collapsed);
  for (QWidget* w : findChildren<QLabel*>()) {
    if (w->objectName() == "sidebarSectionLabel") {
      w->setVisible(!collapsed);
    }
  }

  emit sidebarCollapsedChanged(collapsed);
  emit sidebarWidthChanged(collapsed ? kCollapsedSidebarWidth : kDefaultSidebarWidth);
}

#include "Sidebar.moc"
