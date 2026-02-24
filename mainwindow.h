#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QMap>
#include <QList>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

#include "setup.h"
#include "setupmanager.h"
#include "iracingweek.h"

// ══════════════════════════════════════════════════════════════════════════════
// DatapackCard
// ══════════════════════════════════════════════════════════════════════════════

class DatapackCard : public QWidget
{
    Q_OBJECT

public:
    explicit DatapackCard(const Setup &summary, QWidget *parent = nullptr);

    QString datapackId() const { return m_datapackId; }

    void setDetails(const QList<Setup> &setups);
    void setDownloading(const Setup &setup, qint64 received, qint64 total);
    void setInstalled(const Setup &setup);
    void setFailed(const Setup &setup, const QString &reason);

signals:
    void downloadRequested(const Setup &setup);
    void detailsRequested(const QString &datapackId);

private:
    void buildSummaryUi(const Setup &summary);
    void buildFileRow(const Setup &setup, QWidget *container);

    QString      m_datapackId;
    QLabel      *m_carLabel      = nullptr;
    QLabel      *m_trackLabel    = nullptr;
    QLabel      *m_authorLabel   = nullptr;
    QLabel      *m_seriesLabel   = nullptr;
    QWidget     *m_filesArea     = nullptr;
    QPushButton *m_expandBtn     = nullptr;
    bool         m_detailsLoaded = false;
};

// ══════════════════════════════════════════════════════════════════════════════
// MainWindow
// ══════════════════════════════════════════════════════════════════════════════

class QScrollArea;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onLoginClicked();
    void onRefreshClicked();
    void onSetupListUpdated(const QList<Setup> &setups);
    void onDatapackDetailsLoaded(const QString &datapackId, const QList<Setup> &setups);
    void onDownloadProgress(const Setup &setup, qint64 received, qint64 total);
    void onInstallSucceeded(const Setup &setup, const QString &path);
    void onInstallFailed(const Setup &setup, const QString &reason);
    void onLoginSucceeded();
    void onLoginFailed(const QString &reason);

    void downloadAndInstall(const Setup &setup);

private:
    void setupUi();
    void setupStyleSheet();
    void populateTabs(const QList<Setup> &setups);
    void clearTabs();
    void applyWeekFilter(int week);
    QScrollArea  *getOrCreateTab(const QString &series);
    DatapackCard *findCard(const QString &datapackId);

    QWidget     *m_topBar      = nullptr;
    QLabel      *m_logoLabel   = nullptr;
    QLabel      *m_statusLabel = nullptr;
    QPushButton *m_loginBtn    = nullptr;
    QPushButton *m_refreshBtn  = nullptr;

    QTabWidget  *m_tabs = nullptr;

    QComboBox   *m_weekCombo   = nullptr;
    int          m_currentWeek = 1;

    QMap<QString, QScrollArea *>  m_seriesTabs;
    QMap<QString, DatapackCard *> m_cards;

    SetupManager *m_manager = nullptr;
};
