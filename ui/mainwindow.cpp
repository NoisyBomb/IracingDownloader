#include "mainwindow.h"
#include "registry/trackregistry.h"

#include <QApplication>
#include <QShowEvent>
#include <algorithm>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QComboBox>
#include <QMessageBox>
#include <QPixmap>
#include <QDir>
#include <QSet>
#include <QDebug>


static QPixmap loadPic(const QString &subfolder, const QString &name)
{
    // Looks for :/pic/<subfolder>/<name>.{jpg,png,jpeg}
    const QStringList exts = { "jpg", "png", "jpeg" };
    for (const QString &ext : exts) {
        const QString path = QString("pic/%1/%2.%3").arg(subfolder, name, ext);
        if (QFile::exists(path))
            return QPixmap(path);
    }
    return QPixmap();
}

static QString sanitizeCarName(const QString &s)
{
    QString r = s.toLower();
    r.replace(' ', '_');
    static const QRegularExpression re("[^a-z0-9_]");
    r.remove(re);
    return r;
}

QString DatapackRow::formatLaptime(float seconds)
{
    if (seconds <= 0) return QString();
    const int mins = static_cast<int>(seconds) / 60;
    const float secs = seconds - mins * 60;
    return QString("%1:%2").arg(mins).arg(secs, 6, 'f', 3, '0');
}

void DatapackRow::loadImages()
{
    if (m_imagesLoaded) return;
    m_imagesLoaded = true;

    if (!m_trackFile.isEmpty()) {
        const QPixmap p = loadPic("track", m_trackFile);
        if (!p.isNull())
            m_trackImg->setPixmap(p.scaled(420, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else
            m_trackImg->hide();
    } else {
        m_trackImg->hide();
    }

    if (!m_carFile.isEmpty()) {
        const QPixmap p = loadPic("car", m_carFile);
        if (!p.isNull())
            m_carImg->setPixmap(p.scaled(420, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else
            m_carImg->hide();
    } else {
        m_carImg->hide();
    }
}

DatapackRow::DatapackRow(const Setup &summary, QWidget *parent)
    : QWidget(parent)
    , m_datapackId(summary.id)
{
    setObjectName("DatapackRow");
    buildUi(summary);
}

void DatapackRow::buildUi(const Setup &summary)
{
    // ── Outer layout: left info | center track img | right car img ──
    auto *outer = new QHBoxLayout(this);
    outer->setContentsMargins(16, 12, 16, 12);
    outer->setSpacing(32);
    outer->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // ── LEFT PANEL ──────────────────────────────────────────────────
    auto *leftPanel = new QWidget(this);
    leftPanel->setObjectName("RowLeft");
    leftPanel->setFixedWidth(320);
    leftPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(12, 16, 12, 16);
    leftLayout->setSpacing(4);
    leftLayout->setAlignment(Qt::AlignTop);

    m_seriesLabel = new QLabel(summary.series.toUpper(), leftPanel);
    m_seriesLabel->setObjectName("RowSeriesBadge");
    m_seriesLabel->setMaximumWidth(160);
    m_seriesLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_carLabel = new QLabel(summary.car.displayName, leftPanel);
    m_carLabel->setObjectName("RowCarName");
    m_carLabel->setWordWrap(true);

    m_trackLabel = new QLabel(summary.track.fullName(), leftPanel);
    m_trackLabel->setObjectName("RowTrackName");
    m_trackLabel->setWordWrap(true);

    m_authorLabel = new QLabel("by " + summary.author, leftPanel);
    m_authorLabel->setObjectName("RowAuthor");

    // Laptime
    const QString lt = formatLaptime(summary.laptime);
    m_laptimeLabel = new QLabel(lt.isEmpty() ? "" : "⏱  " + lt, leftPanel);
    m_laptimeLabel->setObjectName("RowLaptime");

    // Wet badge
    if (summary.wet) {
        auto *wetLabel = new QLabel("🌧  WET", leftPanel);
        wetLabel->setObjectName("RowWetBadge");
        leftLayout->addWidget(wetLabel);
    }

    // Files area (expands after Load)
    m_filesArea = new QWidget(leftPanel);
    m_filesArea->setLayout(new QVBoxLayout());
    m_filesArea->layout()->setContentsMargins(0, 4, 0, 0);
    m_filesArea->layout()->setSpacing(4);

    m_loadBtn = new QPushButton("Load setups", leftPanel);
    m_loadBtn->setObjectName("RowLoadBtn");
    connect(m_loadBtn, &QPushButton::clicked, this, [this]() {
        if (!m_detailsLoaded)
            emit detailsRequested(m_datapackId);
        m_loadBtn->setEnabled(false);
        m_loadBtn->setText("Loading…");
    });

    leftLayout->addWidget(m_seriesLabel);
    leftLayout->addSpacing(6);
    leftLayout->addWidget(m_carLabel);
    leftLayout->addWidget(m_trackLabel);
    leftLayout->addWidget(m_authorLabel);
    leftLayout->addWidget(m_laptimeLabel);
    leftLayout->addStretch();
    leftLayout->addWidget(m_filesArea);
    leftLayout->addWidget(m_loadBtn);

    // ── CENTER: track image ──────────────────────────────────────────
    m_trackImg = new QLabel(this);
    m_trackImg->setObjectName("RowTrackImg");
    m_trackImg->setFixedSize(420, 200);
    m_trackImg->setAlignment(Qt::AlignCenter);
    m_trackImg->setScaledContents(false);
    m_trackImg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_trackImg->setMaximumHeight(200);

    // Images loaded lazily via loadImages()
    m_trackFile = TrackRegistry::instance().imageFile(summary.track.displayName);
    m_carFile   = sanitizeCarName(summary.car.displayName);

    // ── RIGHT: car image ─────────────────────────────────────────────
    m_carImg = new QLabel(this);
    m_carImg->setObjectName("RowCarImg");
    m_carImg->setFixedSize(420, 200);
    m_carImg->setAlignment(Qt::AlignCenter);
    m_carImg->setScaledContents(false);
    m_carImg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_carImg->setMaximumHeight(200);

    outer->addWidget(leftPanel, 0, Qt::AlignTop);
    outer->addWidget(m_trackImg, 0, Qt::AlignTop);
    outer->addWidget(m_carImg, 0, Qt::AlignTop);
    outer->addStretch(1);
}

void DatapackRow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadImages();
}

void DatapackRow::setDetails(const QList<Setup> &setups)
{
    if (m_detailsLoaded) return;
    m_detailsLoaded = true;

    // Group: dry setups + wet setups together
    QList<Setup> dry, wet;
    for (const Setup &s : setups) {
        if (s.wet) wet << s;
        else       dry << s;
    }

    buildFileButtons(dry);
    if (!wet.isEmpty()) {
        auto *wetSep = new QLabel("— WET —", m_filesArea);
        wetSep->setObjectName("RowWetSep");
        m_filesArea->layout()->addWidget(wetSep);
        buildFileButtons(wet);
    }

    // Turn load button into collapse/expand toggle
    m_loadBtn->setText("▲  Hide setups");
    m_loadBtn->setEnabled(true);
    disconnect(m_loadBtn, nullptr, nullptr, nullptr);
    connect(m_loadBtn, &QPushButton::clicked, this, [this]() {
        const bool visible = m_filesArea->isVisible();
        m_filesArea->setVisible(!visible);
        m_loadBtn->setText(visible ? "▼  Show setups" : "▲  Hide setups");
    });
}

void DatapackRow::buildFileButtons(const QList<Setup> &setups)
{
    for (const Setup &setup : setups) {
        auto *row = new QWidget(m_filesArea);
        row->setObjectName("FileRow");
        row->setProperty("setupId", setup.id);

        auto *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(6);

        auto *nameLabel = new QLabel(setup.displayName, row);
        nameLabel->setObjectName("FileName");
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        if (setup.wet) {
            auto *wl = new QLabel("🌧", row);
            wl->setObjectName("FileWet");
            hl->addWidget(wl);
        }

        auto *pb = new QProgressBar(row);
        pb->setObjectName("FileProgress");
        pb->setRange(0, 100);
        pb->hide();
        pb->setFixedHeight(4);
        pb->setProperty("setupId", setup.id);

        auto *btn = new QPushButton(setup.isInstalled ? "✓" : "↓", row);
        btn->setObjectName(setup.isInstalled ? "BtnInstalled" : "BtnDownload");
        btn->setFixedWidth(36);
        btn->setEnabled(!setup.isInstalled);
        btn->setProperty("setupId", setup.id);

        connect(btn, &QPushButton::clicked, this, [this, setup, btn, pb]() {
            btn->setEnabled(false);
            btn->setText("…");
            pb->show();
            emit downloadRequested(setup);
        });

        hl->addWidget(nameLabel);
        hl->addWidget(pb);
        hl->addWidget(btn);

        m_filesArea->layout()->addWidget(row);
    }
}

// ── State helpers ──────────────────────────────────────────────────────────

template<typename T>
static T *findBySetupId(QWidget *root, const QString &id)
{
    for (QObject *obj : root->findChildren<QObject *>())
        if (obj->property("setupId").toString() == id)
            if (auto *w = qobject_cast<T *>(obj))
                return w;
    return nullptr;
}

void DatapackRow::setDownloading(const Setup &setup, qint64 received, qint64 total)
{
    if (auto *pb = findBySetupId<QProgressBar>(m_filesArea, setup.id)) {
        pb->show();
        if (total > 0) pb->setValue(static_cast<int>(received * 100 / total));
    }
}

void DatapackRow::setInstalled(const Setup &setup)
{
    if (auto *btn = findBySetupId<QPushButton>(m_filesArea, setup.id)) {
        btn->setText("✓");
        btn->setEnabled(false);
        btn->setStyleSheet("background:#0a2010;color:#40a060;border:1px solid #1a4020;"
                           "border-radius:3px;font-size:13px;");
    }
    if (auto *pb = findBySetupId<QProgressBar>(m_filesArea, setup.id))
        pb->hide();
}

void DatapackRow::setFailed(const Setup &setup, const QString &)
{
    if (auto *btn = findBySetupId<QPushButton>(m_filesArea, setup.id)) {
        btn->setText("↓");
        btn->setEnabled(true);
    }
    if (auto *pb = findBySetupId<QProgressBar>(m_filesArea, setup.id))
        pb->hide();
}

// ══════════════════════════════════════════════════════════════════════════════
// MainWindow
// ══════════════════════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_manager(new SetupManager(this))
{
    setWindowTitle("Grid-and-Go Setup Manager");
    setMinimumSize(1100, 720);
    resize(1280, 800);

    setupUi();
    applyStyleSheet();

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
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Top bar ──────────────────────────────────────────────────────
    m_topBar = new QWidget(central);
    m_topBar->setObjectName("TopBar");
    m_topBar->setFixedHeight(52);

    auto *tl = new QHBoxLayout(m_topBar);
    tl->setContentsMargins(20, 0, 20, 0);
    tl->setSpacing(10);

    m_logoLabel = new QLabel("GRID-AND-GO", m_topBar);
    m_logoLabel->setObjectName("LogoLabel");

    m_statusLabel = new QLabel("", m_topBar);
    m_statusLabel->setObjectName("StatusLabel");

    m_weekCombo = new QComboBox(m_topBar);
    m_weekCombo->setObjectName("WeekCombo");
    m_weekCombo->setFixedWidth(110);
    m_weekCombo->setEnabled(false);
    for (int w = 1; w <= 12; ++w)
        m_weekCombo->addItem(QString("Week %1").arg(w), w);
    m_weekCombo->addItem("Week 13 (Off)", 13);

    m_refreshBtn = new QPushButton("↻  Refresh", m_topBar);
    m_refreshBtn->setObjectName("RefreshBtn");
    m_refreshBtn->setFixedWidth(100);
    m_refreshBtn->setEnabled(false);

    m_loginBtn = new QPushButton("Login", m_topBar);
    m_loginBtn->setObjectName("LoginBtn");
    m_loginBtn->setFixedWidth(100);

    tl->addWidget(m_logoLabel);
    tl->addStretch();
    tl->addWidget(m_statusLabel);
    tl->addWidget(m_weekCombo);
    tl->addWidget(m_refreshBtn);
    tl->addWidget(m_loginBtn);

    // ── Tabs ─────────────────────────────────────────────────────────
    m_tabs = new QTabWidget(central);
    m_tabs->setObjectName("MainTabs");
    m_tabs->setDocumentMode(true);

    auto *ph = new QLabel("Press Login to load setups", m_tabs);
    ph->setObjectName("PlaceholderLabel");
    ph->setAlignment(Qt::AlignCenter);
    m_tabs->addTab(ph, "Welcome");

    root->addWidget(m_topBar);
    root->addWidget(m_tabs, 1);

    connect(m_loginBtn,   &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
}

// ── Auth ───────────────────────────────────────────────────────────────────

void MainWindow::onLoginClicked()
{
    m_loginBtn->setEnabled(false);
    m_loginBtn->setText("…");
    m_manager->login();
}

void MainWindow::onLoginSucceeded()
{
    m_loginBtn->setText("Logout");
    m_loginBtn->setEnabled(true);
    m_refreshBtn->setEnabled(true);

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
        m_weekCombo->setEnabled(false);
        m_statusLabel->setText("");
        disconnect(m_loginBtn, nullptr, this, nullptr);
        connect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
        clearTabs();
    });
}

void MainWindow::onLoginFailed(const QString &reason)
{
    m_loginBtn->setText("Login");
    m_loginBtn->setEnabled(true);
    QMessageBox::warning(this, "Login failed", reason);
}

// ── Data ───────────────────────────────────────────────────────────────────

void MainWindow::onRefreshClicked()
{
    m_refreshBtn->setEnabled(false);
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
    if (DatapackRow *row = findRow(datapackId))
        row->setDetails(setups);
}

void MainWindow::onDownloadProgress(const Setup &setup, qint64 received, qint64 total)
{
    if (DatapackRow *row = findRow(setup.datapackId))
        row->setDownloading(setup, received, total);
}

void MainWindow::onInstallSucceeded(const Setup &setup, const QString &)
{
    if (DatapackRow *row = findRow(setup.datapackId))
        row->setInstalled(setup);
}

void MainWindow::onInstallFailed(const Setup &setup, const QString &reason)
{
    if (DatapackRow *row = findRow(setup.datapackId))
        row->setFailed(setup, reason);
    QMessageBox::warning(this, "Install failed", reason);
}

void MainWindow::downloadAndInstall(const Setup &setup)
{
    m_manager->downloadAndInstall(setup);
}

// ── Tabs ───────────────────────────────────────────────────────────────────

void MainWindow::clearTabs()
{
    m_tabs->clear();
    m_seriesTabs.clear();
    m_rows.clear();
}

QScrollArea *MainWindow::getOrCreateTab(const QString &series)
{
    if (m_seriesTabs.contains(series))
        return m_seriesTabs[series];

    auto *content = new QWidget();
    content->setObjectName("TabContent");
    auto *vl = new QVBoxLayout(content);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(1);
    vl->setAlignment(Qt::AlignTop);

    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setWidget(content);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_tabs->addTab(scroll, series);
    m_seriesTabs[series] = scroll;
    return scroll;
}

void MainWindow::populateTabs(const QList<Setup> &setups)
{
    QMap<QString, QMap<QString, Setup>> bySeries;
    for (const Setup &s : setups)
        if (!bySeries[s.series].contains(s.datapackId))
            bySeries[s.series][s.datapackId] = s;

    for (auto it = bySeries.begin(); it != bySeries.end(); ++it) {
        QScrollArea *scroll = getOrCreateTab(it.key());
        QVBoxLayout *vl     = qobject_cast<QVBoxLayout *>(scroll->widget()->layout());

        QList<Setup> sorted = it.value().values();
        std::sort(sorted.begin(), sorted.end(), [](const Setup &a, const Setup &b) {
            if (a.laptime <= 0) return false;
            if (b.laptime <= 0) return true;
            return a.laptime < b.laptime;
        });

        for (const Setup &summary : sorted) {
            auto *rowWidget = new DatapackRow(summary, scroll->widget());
            rowWidget->setProperty("week", summary.week);

            connect(rowWidget, &DatapackRow::detailsRequested,
                    m_manager,  &SetupManager::loadDatapackDetails);
            connect(rowWidget, &DatapackRow::downloadRequested,
                    this,       &MainWindow::downloadAndInstall);

            auto *sep = new QFrame(scroll->widget());
            sep->setFrameShape(QFrame::HLine);
            sep->setObjectName("RowSeparator");

            vl->addWidget(rowWidget);
            vl->addWidget(sep);

            m_rows[summary.datapackId] = rowWidget;
        }
    }
}

DatapackRow *MainWindow::findRow(const QString &datapackId)
{
    return m_rows.value(datapackId, nullptr);
}

void MainWindow::applyWeekFilter(int week)
{
    for (auto it = m_rows.begin(); it != m_rows.end(); ++it) {
        DatapackRow *row = it.value();
        const int rowWeek = row->property("week").toInt();
        const bool visible = (rowWeek == 0 || rowWeek == week);
        row->setVisible(visible);

        QWidget *par = qobject_cast<QWidget *>(row->parent());
        if (!par) continue;
        QVBoxLayout *vl = qobject_cast<QVBoxLayout *>(par->layout());
        if (!vl) continue;
        const int idx = vl->indexOf(row);
        if (idx >= 0 && idx + 1 < vl->count())
            if (auto *item = vl->itemAt(idx + 1))
                if (auto *sep = qobject_cast<QFrame *>(item->widget()))
                    sep->setVisible(visible);
    }
}

// ── Style ──────────────────────────────────────────────────────────────────

void MainWindow::applyStyleSheet()
{
    qApp->setStyle("Fusion");

    const QString qss = R"(
/* ── BASE ─────────────────────────────────────────────────────────── */
QWidget {
    background-color: #0A0E1A;
    color: #94A3B8;
    font-family: "Segoe UI", "Inter", sans-serif;
    font-size: 13px;
}

/* ── TOP BAR ──────────────────────────────────────────────────────── */
#TopBar {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #0A0E1A, stop:0.5 #0f1525, stop:1 #0A0E1A);
    border-bottom: 1px solid rgba(225,6,0,0.3);
}
#LogoLabel {
    color: #E10600;
    font-size: 15px;
    font-weight: 800;
    letter-spacing: 5px;
}
#StatusLabel {
    color: #4a6080;
    font-size: 11px;
    letter-spacing: 1px;
    background: transparent;
}

/* ── BUTTONS ──────────────────────────────────────────────────────── */
QPushButton {
    background-color: rgba(255,255,255,0.04);
    color: #64748B;
    border: 1px solid rgba(255,255,255,0.08);
    border-radius: 5px;
    padding: 5px 14px;
    font-size: 12px;
}
QPushButton:hover {
    background-color: rgba(225,6,0,0.08);
    color: #94A3B8;
    border-color: rgba(225,6,0,0.3);
}
QPushButton:pressed { background-color: rgba(225,6,0,0.15); }
QPushButton:disabled { color: #1e2a3a; border-color: rgba(255,255,255,0.03); }

#LoginBtn {
    background: rgba(225,6,0,0.12);
    color: #E10600;
    border: 1px solid rgba(225,6,0,0.3);
    font-weight: 600;
}
#LoginBtn:hover {
    background: rgba(225,6,0,0.22);
    color: #ff3020;
    border-color: rgba(225,6,0,0.6);
}
#RefreshBtn {
    background: rgba(0,217,255,0.06);
    color: #00D9FF;
    border: 1px solid rgba(0,217,255,0.2);
}
#RefreshBtn:hover {
    background: rgba(0,217,255,0.12);
    border-color: rgba(0,217,255,0.4);
}

/* ── WEEK COMBO ───────────────────────────────────────────────────── */
#WeekCombo {
    background: rgba(255,255,255,0.04);
    color: #FF6B35;
    border: 1px solid rgba(255,107,53,0.3);
    border-radius: 5px;
    padding: 4px 10px;
    font-weight: 700;
    font-size: 12px;
}
#WeekCombo:disabled { color: #1e2a3a; border-color: rgba(255,255,255,0.03); }
QComboBox QAbstractItemView {
    background: #0f1525;
    color: #94A3B8;
    border: 1px solid rgba(225,6,0,0.2);
    selection-background-color: rgba(225,6,0,0.15);
}

/* ── TABS ─────────────────────────────────────────────────────────── */
QTabWidget::pane {
    border: none;
    border-top: 1px solid rgba(255,255,255,0.06);
}
QTabBar { background: transparent; }
QTabBar::tab {
    background: transparent;
    color: rgba(148,163,184,0.5);
    padding: 10px 20px;
    border: none;
    border-bottom: 2px solid transparent;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 1px;
    min-width: 80px;
}
QTabBar::tab:selected {
    color: #FFFFFF;
    border-bottom: 2px solid #E10600;
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
        stop:0 rgba(225,6,0,0.08), stop:1 transparent);
}
QTabBar::tab:hover:!selected {
    color: rgba(148,163,184,0.9);
    border-bottom: 2px solid rgba(225,6,0,0.3);
}

/* ── SCROLL ───────────────────────────────────────────────────────── */
QScrollArea { background: #0A0E1A; border: none; }
QScrollBar:vertical {
    background: transparent;
    width: 4px;
    margin: 0;
}
QScrollBar::handle:vertical {
    background: rgba(225,6,0,0.3);
    border-radius: 2px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover { background: rgba(225,6,0,0.6); }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

/* ── ROW CARD ─────────────────────────────────────────────────────── */
#DatapackRow {
    background: rgba(21,27,40,0.6);
    border-bottom: 1px solid rgba(255,255,255,0.04);
    min-height: 200px;
}
#DatapackRow:hover {
    background: rgba(225,6,0,0.04);
    border-bottom: 1px solid rgba(225,6,0,0.15);
}

#RowSeparator {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 transparent,
        stop:0.2 rgba(225,6,0,0.2),
        stop:0.8 rgba(225,6,0,0.2),
        stop:1 transparent);
    border: none;
    max-height: 1px;
}

/* ── LEFT PANEL ───────────────────────────────────────────────────── */
#RowLeft { background: transparent; }

#RowSeriesBadge {
    color: #E10600;
    font-size: 9px;
    font-weight: 800;
    letter-spacing: 3px;
    background: rgba(225,6,0,0.1);
    border: 1px solid rgba(225,6,0,0.25);
    border-radius: 3px;
    padding: 2px 8px;
}
#RowCarName {
    color: #FFFFFF;
    font-size: 17px;
    font-weight: 700;
}
#RowTrackName {
    color: #64748B;
    font-size: 13px;
}
#RowAuthor {
    color: #334155;
    font-size: 11px;
    font-style: italic;
}
#RowLaptime {
    color: #00FF88;
    font-size: 13px;
    font-family: "Consolas", "JetBrains Mono", monospace;
    font-weight: 600;
}
#RowWetBadge {
    color: #00D9FF;
    font-size: 11px;
    font-weight: 700;
    background: rgba(0,217,255,0.08);
    border: 1px solid rgba(0,217,255,0.2);
    border-radius: 3px;
    padding: 1px 6px;
}
#RowWetSep {
    color: #00D9FF;
    font-size: 10px;
    font-style: italic;
    letter-spacing: 2px;
}

/* ── IMAGES ───────────────────────────────────────────────────────── */
#RowTrackImg, #RowCarImg {
    background: transparent;
    border: none;
}

/* ── LOAD / TOGGLE BTN ────────────────────────────────────────────── */
#RowLoadBtn {
    background: rgba(225,6,0,0.12);
    color: #E10600;
    border: 1px solid rgba(225,6,0,0.4);
    border-radius: 4px;
    padding: 6px;
    font-size: 12px;
    font-weight: 600;
    letter-spacing: 0.5px;
}
#RowLoadBtn:hover {
    background: rgba(225,6,0,0.22);
    color: #ff3020;
    border-color: rgba(225,6,0,0.7);
}

/* ── FILE ROWS ────────────────────────────────────────────────────── */
#FileRow { background: transparent; }
#FileName { color: #475569; font-size: 12px; }
#FileWet  { color: #00D9FF; }

#BtnDownload {
    background: rgba(0,217,255,0.08);
    color: #00D9FF;
    border: 1px solid rgba(0,217,255,0.25);
    border-radius: 4px;
    font-size: 14px;
    font-weight: 700;
    padding: 2px;
}
#BtnDownload:hover {
    background: rgba(0,217,255,0.18);
    border-color: rgba(0,217,255,0.5);
    color: #40eeff;
}
#BtnInstalled {
    background: rgba(0,255,136,0.08);
    color: #00FF88;
    border: 1px solid rgba(0,255,136,0.25);
    border-radius: 4px;
    font-size: 13px;
    padding: 2px;
}

QProgressBar#FileProgress {
    background: rgba(255,255,255,0.05);
    border: none;
    border-radius: 2px;
}
QProgressBar#FileProgress::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #E10600, stop:1 #FF6B35);
    border-radius: 2px;
}

/* ── PLACEHOLDER ──────────────────────────────────────────────────── */
#PlaceholderLabel {
    color: rgba(225,6,0,0.15);
    font-size: 22px;
    font-weight: 700;
    letter-spacing: 4px;
}
    )";

    qApp->setStyleSheet(qss);
}
