#include "mainwindow.h"
#include "trackregistry.h"

#include <QApplication>
#include <algorithm>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QComboBox>
#include <QMessageBox>
#include <QPixmap>
#include <QDir>
#include <QSet>
#include <QDebug>

// ══════════════════════════════════════════════════════════════════════════════
// Helpers
// ══════════════════════════════════════════════════════════════════════════════

static QPixmap loadPic(const QString &subfolder, const QString &name)
{
    // Looks for pic/<subfolder>/<name>.{jpg,png,jpeg}
    const QStringList exts = { "jpg", "png", "jpeg" };
    for (const QString &ext : exts) {
        const QString path = QString("pic/%1/%2.%3").arg(subfolder, name, ext);
        if (QFile::exists(path))
            return QPixmap(path);
        qDebug() << "[loadPic] cwd:" << QDir::currentPath() << "path:" << path;
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

// ══════════════════════════════════════════════════════════════════════════════
// DatapackRow
// ══════════════════════════════════════════════════════════════════════════════

QString DatapackRow::formatLaptime(float seconds)
{
    if (seconds <= 0) return QString();
    const int mins = static_cast<int>(seconds) / 60;
    const float secs = seconds - mins * 60;
    return QString("%1:%2").arg(mins).arg(secs, 6, 'f', 3, '0');
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
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // ── LEFT PANEL ──────────────────────────────────────────────────
    auto *leftPanel = new QWidget(this);
    leftPanel->setObjectName("RowLeft");
    leftPanel->setFixedWidth(340);

    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(4);

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
    m_trackImg->setFixedSize(380, 210);
    m_trackImg->setAlignment(Qt::AlignCenter);
    m_trackImg->setScaledContents(false);

    const QString trackFile = TrackRegistry::instance().imageFile(summary.track.displayName);
    const QPixmap trackPix = loadPic("track", trackFile);
    if (!trackPix.isNull())
        m_trackImg->setPixmap(trackPix.scaled(380, 210, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // ── RIGHT: car image ─────────────────────────────────────────────
    m_carImg = new QLabel(this);
    m_carImg->setObjectName("RowCarImg");
    m_carImg->setFixedSize(380, 210);
    m_carImg->setAlignment(Qt::AlignCenter);
    m_carImg->setScaledContents(false);

    const QPixmap carPix = loadPic("car", sanitizeCarName(summary.car.displayName));
    if (!carPix.isNull())
        m_carImg->setPixmap(carPix.scaled(380, 210, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    outer->addWidget(leftPanel);
    outer->addWidget(m_trackImg, 1);
    outer->addWidget(m_carImg, 1);
}

void DatapackRow::setDetails(const QList<Setup> &setups)
{
    if (m_detailsLoaded) return;
    m_detailsLoaded = true;
    m_loadBtn->hide();

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
    // Group by series, deduplicate by datapackId
    QMap<QString, QMap<QString, Setup>> bySeries;
    for (const Setup &s : setups)
        if (!bySeries[s.series].contains(s.datapackId))
            bySeries[s.series][s.datapackId] = s;

    for (auto it = bySeries.begin(); it != bySeries.end(); ++it) {
        QScrollArea *scroll = getOrCreateTab(it.key());
        QVBoxLayout *vl     = qobject_cast<QVBoxLayout *>(scroll->widget()->layout());

        // Sort by laptime ascending (fastest first), zeros go last
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

            // Separator line
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
        row->setVisible(rowWeek == 0 || rowWeek == week);

        // Also hide/show the separator that follows the row
        // Separator is the next sibling widget
        QWidget *parent = qobject_cast<QWidget *>(row->parent());
        if (!parent) continue;
        QVBoxLayout *vl = qobject_cast<QVBoxLayout *>(parent->layout());
        if (!vl) continue;
        const int idx = vl->indexOf(row);
        if (idx >= 0 && idx + 1 < vl->count()) {
            if (auto *item = vl->itemAt(idx + 1))
                if (auto *sep = qobject_cast<QFrame *>(item->widget()))
                    sep->setVisible(row->isVisible());
        }
    }
}

// ── Style ──────────────────────────────────────────────────────────────────

void MainWindow::applyStyleSheet()
{
    qApp->setStyle("Fusion");

    const QString qss = R"(
QWidget {
    background-color: #0d1117;
    color: #c9d1d9;
    font-family: "Segoe UI", "SF Pro Display", sans-serif;
    font-size: 13px;
}

/* Top bar */
#TopBar {
    background-color: #080c12;
    border-bottom: 1px solid #1a2030;
}
#LogoLabel {
    color: #c8a84b;
    font-size: 14px;
    font-weight: 700;
    letter-spacing: 4px;
}
#StatusLabel {
    color: #3d5070;
    font-size: 12px;
}

/* Buttons */
QPushButton {
    background-color: #161c28;
    color: #7090b0;
    border: 1px solid #202c40;
    border-radius: 4px;
    padding: 5px 14px;
    font-size: 12px;
}
QPushButton:hover  { background-color: #1a2235; color: #a0b8d0; border-color: #304060; }
QPushButton:pressed { background-color: #111825; }
QPushButton:disabled { color: #2a3545; border-color: #161c28; }

#LoginBtn  { background-color: #0f2040; color: #4878c0; border-color: #1a3060; }
#LoginBtn:hover { background-color: #132850; color: #6898e0; }
#RefreshBtn { background-color: #101810; color: #406840; border-color: #1a2c1a; }
#RefreshBtn:hover { background-color: #142014; color: #60a060; }

/* Week combo */
#WeekCombo {
    background-color: #161c28;
    color: #c8a84b;
    border: 1px solid #2a3020;
    border-radius: 4px;
    padding: 4px 10px;
    font-weight: 600;
    font-size: 12px;
}
#WeekCombo:disabled { color: #2a3040; }
QComboBox QAbstractItemView {
    background-color: #0d1117;
    color: #c9d1d9;
    border: 1px solid #202c40;
    selection-background-color: #1a2235;
}

/* Tabs */
QTabWidget::pane { border: none; }
QTabBar { background-color: #080c12; }
QTabBar::tab {
    background: transparent;
    color: #3d5070;
    padding: 9px 18px;
    border: none;
    border-bottom: 2px solid transparent;
    font-size: 11px;
    font-weight: 600;
    letter-spacing: 0.5px;
}
QTabBar::tab:selected { color: #c8a84b; border-bottom: 2px solid #c8a84b; }
QTabBar::tab:hover:!selected { color: #6080a0; }

/* Scroll */
QScrollArea { background: #0d1117; border: none; }
QScrollBar:vertical { background: #0d1117; width: 5px; }
QScrollBar::handle:vertical { background: #1e2c40; border-radius: 2px; min-height: 30px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

/* Row */
#DatapackRow {
    background-color: #0d1117;
    min-height: 200px;
    max-height: 300px;
}
#DatapackRow:hover { background-color: #0f1520; }

#RowSeparator { color: #141c28; }

#RowLeft { background-color: transparent; }

#RowSeriesBadge {
    color: #c8a84b;
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 2px;
    background-color: #1c1500;
    border-radius: 3px;
    padding: 2px 8px;
}
#RowCarName  { color: #e6edf3; font-size: 16px; font-weight: 600; }
#RowTrackName { color: #6080a0; font-size: 13px; }
#RowAuthor   { color: #3d5070; font-size: 11px; font-style: italic; }
#RowLaptime  { color: #508060; font-size: 12px; font-family: "Consolas", monospace; }
#RowWetBadge { color: #4090c0; font-size: 11px; font-weight: 600; }
#RowWetSep   { color: #3d5070; font-size: 10px; font-style: italic; }

/* Images */
#RowTrackImg, #RowCarImg { background-color: #080c12; }

/* Load btn */
#RowLoadBtn {
    background-color: #0d1520;
    color: #3d5878;
    border: 1px dashed #1a2838;
    border-radius: 3px;
    padding: 4px;
    font-size: 11px;
}
#RowLoadBtn:hover { color: #5878a0; border-color: #2a3c58; }

/* File rows */
#FileRow  { background: transparent; }
#FileName { color: #7090a8; font-size: 12px; }
#FileWet  { color: #4090c0; }

#BtnDownload {
    background-color: #081828;
    color: #3868a8;
    border: 1px solid #102038;
    border-radius: 3px;
    font-size: 14px;
    font-weight: 600;
    padding: 2px;
}
#BtnDownload:hover { background-color: #0c2040; color: #5888c8; }
#BtnInstalled {
    background-color: #081808;
    color: #388060;
    border: 1px solid #103020;
    border-radius: 3px;
    font-size: 13px;
    padding: 2px;
}

QProgressBar#FileProgress {
    background: #1a2030; border: none; border-radius: 2px;
}
QProgressBar#FileProgress::chunk { background: #3868a8; border-radius: 2px; }

#PlaceholderLabel { color: #1e2c40; font-size: 20px; letter-spacing: 2px; }
    )";

    qApp->setStyleSheet(qss);
}
