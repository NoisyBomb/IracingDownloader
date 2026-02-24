#include "mainwindow.h"

#include <QApplication>
#include <QComboBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>

// ══════════════════════════════════════════════════════════════════════════════
// DatapackCard — implementation
// ══════════════════════════════════════════════════════════════════════════════

DatapackCard::DatapackCard(const Setup &summary, QWidget *parent)
    : QWidget(parent)
    , m_datapackId(summary.id)
{
    setObjectName("DatapackCard");
    buildSummaryUi(summary);
}

void DatapackCard::buildSummaryUi(const Setup &summary)
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(6);
    root->setContentsMargins(14, 14, 14, 14);

    m_seriesLabel = new QLabel(summary.series.toUpper(), this);
    m_seriesLabel->setObjectName("SeriesBadge");

    m_carLabel = new QLabel(summary.car.displayName, this);
    m_carLabel->setObjectName("CardCarName");
    m_carLabel->setWordWrap(true);

    m_trackLabel = new QLabel(summary.track.fullName(), this);
    m_trackLabel->setObjectName("CardTrackName");
    m_trackLabel->setWordWrap(true);

    m_authorLabel = new QLabel("by " + summary.author, this);
    m_authorLabel->setObjectName("CardAuthor");

    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("CardSeparator");

    m_filesArea = new QWidget(this);
    m_filesArea->setLayout(new QVBoxLayout());
    m_filesArea->layout()->setContentsMargins(0, 0, 0, 0);
    m_filesArea->layout()->setSpacing(4);

    m_expandBtn = new QPushButton("Load setups", this);
    m_expandBtn->setObjectName("CardExpandBtn");
    connect(m_expandBtn, &QPushButton::clicked, this, [this]() {
        if (!m_detailsLoaded)
            emit detailsRequested(m_datapackId);
    });

    root->addWidget(m_seriesLabel);
    root->addWidget(m_carLabel);
    root->addWidget(m_trackLabel);
    root->addWidget(m_authorLabel);
    root->addWidget(sep);
    root->addWidget(m_filesArea);
    root->addWidget(m_expandBtn);
    root->addStretch();
}

void DatapackCard::setDetails(const QList<Setup> &setups)
{
    if (m_detailsLoaded)
        return;
    m_detailsLoaded = true;
    m_expandBtn->hide();

    for (const Setup &s : setups)
        buildFileRow(s, m_filesArea);
}

void DatapackCard::buildFileRow(const Setup &setup, QWidget *container)
{
    auto *row = new QWidget(container);
    row->setObjectName("FileRow");
    row->setProperty("setupId", setup.id);

    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(8);

    auto *nameLabel = new QLabel(setup.displayName, row);
    nameLabel->setObjectName("FileName");
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *progressBar = new QProgressBar(row);
    progressBar->setObjectName("FileProgress");
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->hide();
    progressBar->setFixedHeight(4);
    progressBar->setProperty("setupId", setup.id);

    auto *dlBtn = new QPushButton(setup.isInstalled ? "✓ Installed" : "↓", row);
    dlBtn->setObjectName(setup.isInstalled ? "BtnInstalled" : "BtnDownload");
    dlBtn->setFixedWidth(80);
    dlBtn->setProperty("setupId", setup.id);
    dlBtn->setEnabled(!setup.isInstalled);

    connect(dlBtn, &QPushButton::clicked, this, [this, setup, dlBtn, progressBar]() {
        dlBtn->setEnabled(false);
        dlBtn->setText("…");
        progressBar->show();
        emit downloadRequested(setup);
    });

    hl->addWidget(nameLabel);
    hl->addWidget(progressBar);
    hl->addWidget(dlBtn);

    container->layout()->addWidget(row);
}

// ── State helpers ──────────────────────────────────────────────────────────

template<typename T>
static T *findBySetupId(QWidget *root, const QString &setupId)
{
    for (QObject *obj : root->findChildren<QObject *>()) {
        if (obj->property("setupId").toString() == setupId)
            if (auto *w = qobject_cast<T *>(obj))
                return w;
    }
    return nullptr;
}

void DatapackCard::setDownloading(const Setup &setup, qint64 received, qint64 total)
{
    if (auto *pb = findBySetupId<QProgressBar>(m_filesArea, setup.id)) {
        pb->show();
        if (total > 0)
            pb->setValue(static_cast<int>(received * 100 / total));
    }
}

void DatapackCard::setInstalled(const Setup &setup)
{
    if (auto *btn = findBySetupId<QPushButton>(m_filesArea, setup.id)) {
        btn->setText("✓ Installed");
        btn->setEnabled(false);
        btn->setStyleSheet("background-color:#0a2010;color:#40a060;"
                           "border:1px solid #1a4020;border-radius:4px;"
                           "padding:3px 8px;font-size:12px;");
    }
    if (auto *pb = findBySetupId<QProgressBar>(m_filesArea, setup.id))
        pb->hide();
}

void DatapackCard::setFailed(const Setup &setup, const QString &reason)
{
    Q_UNUSED(reason)
    if (auto *btn = findBySetupId<QPushButton>(m_filesArea, setup.id)) {
        btn->setText("Retry");
        btn->setEnabled(true);
    }
    if (auto *pb = findBySetupId<QProgressBar>(m_filesArea, setup.id))
        pb->hide();
}

// ══════════════════════════════════════════════════════════════════════════════
// MainWindow — implementation
// ══════════════════════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_manager(new SetupManager(this))
{
    setWindowTitle("Grid-and-Go Setup Manager");
    setMinimumSize(1100, 720);
    resize(1280, 800);

    setupUi();
    setupStyleSheet();

    connect(m_manager, &SetupManager::loginSucceeded,        this, &MainWindow::onLoginSucceeded);
    connect(m_manager, &SetupManager::loginFailed,           this, &MainWindow::onLoginFailed);
    connect(m_manager, &SetupManager::setupListUpdated,      this, &MainWindow::onSetupListUpdated);
    connect(m_manager, &SetupManager::datapackDetailsLoaded, this, &MainWindow::onDatapackDetailsLoaded);
    connect(m_manager, &SetupManager::downloadProgress,      this, &MainWindow::onDownloadProgress);
    connect(m_manager, &SetupManager::installSucceeded,      this, &MainWindow::onInstallSucceeded);
    connect(m_manager, &SetupManager::installFailed,         this, &MainWindow::onInstallFailed);
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Top bar ──
    m_topBar = new QWidget(central);
    m_topBar->setObjectName("TopBar");
    m_topBar->setFixedHeight(56);

    auto *topLayout = new QHBoxLayout(m_topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);
    topLayout->setSpacing(12);

    m_logoLabel = new QLabel("GRID-AND-GO", m_topBar);
    m_logoLabel->setObjectName("LogoLabel");

    m_statusLabel = new QLabel("Not logged in", m_topBar);
    m_statusLabel->setObjectName("StatusLabel");

    m_loginBtn = new QPushButton("Login", m_topBar);
    m_loginBtn->setObjectName("LoginBtn");
    m_loginBtn->setFixedWidth(100);

    m_refreshBtn = new QPushButton("↻ Refresh", m_topBar);
    m_refreshBtn->setObjectName("RefreshBtn");
    m_refreshBtn->setFixedWidth(100);
    m_refreshBtn->setEnabled(false);

    m_weekCombo = new QComboBox(m_topBar);
    m_weekCombo->setObjectName("WeekCombo");
    m_weekCombo->setFixedWidth(110);
    m_weekCombo->setEnabled(false);
    for (int w = 1; w <= 12; ++w)
        m_weekCombo->addItem(QString("Week %1").arg(w), w);

    topLayout->addWidget(m_logoLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_statusLabel);
    topLayout->addWidget(m_weekCombo);
    topLayout->addWidget(m_refreshBtn);
    topLayout->addWidget(m_loginBtn);

    // ── Tab widget ──
    m_tabs = new QTabWidget(central);
    m_tabs->setObjectName("MainTabs");
    m_tabs->setDocumentMode(true);

    auto *placeholder = new QLabel("Login to load setups", m_tabs);
    placeholder->setObjectName("PlaceholderLabel");
    placeholder->setAlignment(Qt::AlignCenter);
    m_tabs->addTab(placeholder, "Welcome");

    rootLayout->addWidget(m_topBar);
    rootLayout->addWidget(m_tabs, 1);

    connect(m_loginBtn,   &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
}

// ── Auth slots ─────────────────────────────────────────────────────────────

void MainWindow::onLoginClicked()
{
    m_loginBtn->setEnabled(false);
    m_loginBtn->setText("…");
    m_statusLabel->setText("Authenticating…");
    m_manager->login();
}

void MainWindow::onLoginSucceeded()
{
    m_loginBtn->setText("Logout");
    m_loginBtn->setEnabled(true);
    m_refreshBtn->setEnabled(true);
    m_statusLabel->setText("Logged in");

    // Выставляем текущую неделю
    const IracingWeek cur = IracingWeek::current();
    m_currentWeek = cur.week;
    m_weekCombo->setCurrentIndex(cur.week - 1);
    m_weekCombo->setEnabled(true);
    connect(m_weekCombo, &QComboBox::currentIndexChanged, this, [this](int idx) {
        m_currentWeek = idx + 1;
        applyWeekFilter(m_currentWeek);
    });

    disconnect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_loginBtn, &QPushButton::clicked, this, [this]() {
        m_manager->logout();
        m_loginBtn->setText("Login");
        m_refreshBtn->setEnabled(false);
        m_statusLabel->setText("Not logged in");
        m_weekCombo->setEnabled(false);
        disconnect(m_loginBtn, nullptr, this, nullptr);
        connect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
        clearTabs();
    });
}

void MainWindow::onLoginFailed(const QString &reason)
{
    m_loginBtn->setText("Login");
    m_loginBtn->setEnabled(true);
    m_statusLabel->setText("Login failed");
    QMessageBox::warning(this, "Login failed", reason);
}

// ── Data slots ─────────────────────────────────────────────────────────────

void MainWindow::onRefreshClicked()
{
    m_refreshBtn->setEnabled(false);
    m_statusLabel->setText("Loading setups…");
    clearTabs();
    m_manager->refreshDatapackList();
}

void MainWindow::onSetupListUpdated(const QList<Setup> &setups)
{
    m_refreshBtn->setEnabled(true);
    m_statusLabel->setText(QString("%1 datapacks").arg(setups.size()));
    populateTabs(setups);
    applyWeekFilter(m_currentWeek);
}

void MainWindow::onDatapackDetailsLoaded(const QString &datapackId, const QList<Setup> &setups)
{
    if (DatapackCard *card = findCard(datapackId))
        card->setDetails(setups);
}

void MainWindow::onDownloadProgress(const Setup &setup, qint64 received, qint64 total)
{
    if (DatapackCard *card = findCard(setup.datapackId))
        card->setDownloading(setup, received, total);
}

void MainWindow::onInstallSucceeded(const Setup &setup, const QString &path)
{
    Q_UNUSED(path)
    if (DatapackCard *card = findCard(setup.datapackId))
        card->setInstalled(setup);
}

void MainWindow::onInstallFailed(const Setup &setup, const QString &reason)
{
    if (DatapackCard *card = findCard(setup.datapackId))
        card->setFailed(setup, reason);
    QMessageBox::warning(this, "Install failed", reason);
}

void MainWindow::downloadAndInstall(const Setup &setup)
{
    m_manager->downloadAndInstall(setup);
}

// ── Tab / card management ──────────────────────────────────────────────────

void MainWindow::clearTabs()
{
    m_tabs->clear();
    m_seriesTabs.clear();
    m_cards.clear();
}

QScrollArea *MainWindow::getOrCreateTab(const QString &series)
{
    if (m_seriesTabs.contains(series))
        return m_seriesTabs[series];

    auto *content = new QWidget();
    content->setObjectName("TabContent");

    auto *grid = new QGridLayout(content);
    grid->setSpacing(12);
    grid->setContentsMargins(16, 16, 16, 16);
    grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setWidget(content);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setObjectName("TabScroll");

    m_tabs->addTab(scroll, series);
    m_seriesTabs[series] = scroll;
    return scroll;
}

void MainWindow::populateTabs(const QList<Setup> &setups)
{
    QMap<QString, QMap<QString, Setup>> bySeries;
    for (const Setup &s : setups) {
        if (!bySeries[s.series].contains(s.datapackId))
            bySeries[s.series][s.datapackId] = s;
    }

    const int columns = 3;

    for (auto it = bySeries.begin(); it != bySeries.end(); ++it) {
        QScrollArea *scroll = getOrCreateTab(it.key());
        QGridLayout *grid   = qobject_cast<QGridLayout *>(scroll->widget()->layout());

        int row = 0, col = 0;
        for (const Setup &summary : it.value()) {
            auto *card = new DatapackCard(summary, scroll->widget());

            connect(card, &DatapackCard::detailsRequested,
                    m_manager, &SetupManager::loadDatapackDetails);
            connect(card, &DatapackCard::downloadRequested,
                    this, &MainWindow::downloadAndInstall);

            card->setProperty("week", summary.week);
            grid->addWidget(card, row, col);
            m_cards[summary.datapackId] = card;

            if (++col >= columns) { col = 0; ++row; }
        }
    }
}

DatapackCard *MainWindow::findCard(const QString &datapackId)
{
    return m_cards.value(datapackId, nullptr);
}


void MainWindow::applyWeekFilter(int week)
{
    for (auto it = m_cards.begin(); it != m_cards.end(); ++it) {
        DatapackCard *card = it.value();
        // DatapackCard хранит week в dynamic property, выставленном при создании
        const int cardWeek = card->property("week").toInt();
        card->setVisible(cardWeek == 0 || cardWeek == week);
    }
}

// ── Style Sheet ────────────────────────────────────────────────────────────

void MainWindow::setupStyleSheet()
{
    qApp->setStyle("Fusion");

    const QString qss = R"(
QWidget {
    background-color: #0f1117;
    color: #d0d6e0;
    font-family: "Segoe UI", "SF Pro Display", sans-serif;
    font-size: 13px;
}
#TopBar {
    background-color: #080b10;
    border-bottom: 1px solid #1e2535;
}
#LogoLabel {
    color: #c8a84b;
    font-size: 15px;
    font-weight: 700;
    letter-spacing: 3px;
}
#StatusLabel {
    color: #5a6478;
    font-size: 12px;
}
QPushButton {
    background-color: #1a2030;
    color: #8fa0c0;
    border: 1px solid #2a3348;
    border-radius: 5px;
    padding: 6px 14px;
    font-weight: 500;
}
QPushButton:hover {
    background-color: #1e2a40;
    color: #c8d4e8;
    border-color: #3a4a68;
}
QPushButton:pressed { background-color: #151e30; }
QPushButton:disabled { color: #3a4458; border-color: #1a2030; }
#LoginBtn { background-color: #1a3060; color: #6090e0; border-color: #2a4080; }
#LoginBtn:hover { background-color: #1e3870; color: #80b0ff; }
#WeekCombo {
    background-color: #1a2030;
    color: #c8a84b;
    border: 1px solid #2a3a20;
    border-radius: 5px;
    padding: 4px 10px;
    font-weight: 600;
    font-size: 12px;
}
#WeekCombo:disabled { color: #3a4458; border-color: #1a2030; }
#WeekCombo QAbstractItemView {
    background-color: #131820;
    color: #c8d4e8;
    border: 1px solid #2a3448;
    selection-background-color: #1e2a40;
}
QTabWidget::pane { border: none; background-color: #0f1117; }
QTabBar { background-color: #080b10; }
QTabBar::tab {
    background-color: transparent;
    color: #4a5878;
    padding: 10px 20px;
    border: none;
    border-bottom: 2px solid transparent;
    font-weight: 500;
    letter-spacing: 1px;
    font-size: 11px;
}
QTabBar::tab:selected { color: #c8a84b; border-bottom: 2px solid #c8a84b; }
QTabBar::tab:hover:!selected { color: #8090b0; }
QScrollArea, #TabScroll { background-color: #0f1117; border: none; }
QScrollBar:vertical { background: #0f1117; width: 6px; margin: 0; }
QScrollBar::handle:vertical { background: #2a3448; border-radius: 3px; min-height: 30px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
#DatapackCard {
    background-color: #131820;
    border: 1px solid #1e2838;
    border-radius: 8px;
    min-width: 300px;
    max-width: 400px;
}
#DatapackCard:hover { border-color: #2a3a58; background-color: #161d28; }
#SeriesBadge {
    color: #c8a84b;
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 2px;
    background-color: #1e1800;
    border-radius: 3px;
    padding: 2px 8px;
}
#CardCarName { color: #e0e8f8; font-size: 15px; font-weight: 600; }
#CardTrackName { color: #7888a8; font-size: 12px; }
#CardAuthor { color: #4a5878; font-size: 11px; font-style: italic; }
#CardSeparator { color: #1e2838; }
#FileRow { background-color: transparent; }
#FileName { color: #8898b8; font-size: 12px; }
#CardExpandBtn {
    background-color: #0f1820;
    color: #4a6090;
    border: 1px dashed #1e2e48;
    border-radius: 4px;
    padding: 5px;
    font-size: 11px;
}
#CardExpandBtn:hover { border-color: #3a5080; color: #6080b0; background-color: #111e30; }
#BtnDownload {
    background-color: #0a2040;
    color: #4080c0;
    border: 1px solid #1a3060;
    border-radius: 4px;
    padding: 3px 8px;
    font-size: 12px;
    font-weight: 600;
}
#BtnDownload:hover { background-color: #0f2a58; color: #60a0e0; }
#BtnInstalled {
    background-color: #0a2010;
    color: #40a060;
    border: 1px solid #1a4020;
    border-radius: 4px;
    padding: 3px 8px;
    font-size: 12px;
}
QProgressBar#FileProgress {
    background-color: #1a2030;
    border: none;
    border-radius: 2px;
}
QProgressBar#FileProgress::chunk { background-color: #4080c0; border-radius: 2px; }
#PlaceholderLabel { color: #2a3448; font-size: 18px; font-weight: 300; letter-spacing: 2px; }
    )";

    qApp->setStyleSheet(qss);
}
