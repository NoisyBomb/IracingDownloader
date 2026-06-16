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
#include "models/setup.h"
#include "core/setupmanager.h"
#include "models/iracingweek.h"


class DatapackRow : public QWidget
{
    Q_OBJECT

public:
    explicit DatapackRow(const Setup &summary, QWidget *parent = nullptr);

    QString datapackId() const { return m_datapackId; }

    void setDetails(const QList<Setup> &setups);
    void setDownloading(const Setup &setup, qint64 received, qint64 total);
    void setInstalled(const Setup &setup);
    void setFailed(const Setup &setup, const QString &reason);

signals:
    void downloadRequested(const Setup &setup);
    void detailsRequested(const QString &datapackId);

private:
    void buildUi(const Setup &summary);
    void buildFileButtons(const QList<Setup> &setups);

    QString      m_datapackId;

    QLabel      *m_seriesLabel  = nullptr;
    QLabel      *m_carLabel     = nullptr;
    QLabel      *m_trackLabel   = nullptr;
    QLabel      *m_authorLabel  = nullptr;
    QLabel      *m_laptimeLabel = nullptr;

    QLabel      *m_trackImg     = nullptr;
    QString      m_trackFile;

    QLabel      *m_carImg       = nullptr;
    QString      m_carFile;
    bool         m_imagesLoaded = false;

    QWidget     *m_filesArea    = nullptr;
    QPushButton *m_loadBtn      = nullptr;
    bool         m_detailsLoaded = false;

    void loadImages();
    void showEvent(QShowEvent *event) override;
    static QString formatLaptime(float seconds);
};

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
    void applyStyleSheet();
    void populateTabs(const QList<Setup> &setups);
    void clearTabs();
    QScrollArea *getOrCreateTab(const QString &series);
    DatapackRow *findRow(const QString &datapackId);
    void applyWeekFilter(int week);

    // Top bar
    QWidget     *m_topBar      = nullptr;
    QLabel      *m_logoLabel   = nullptr;
    QLabel      *m_statusLabel = nullptr;
    QComboBox   *m_weekCombo   = nullptr;
    QPushButton *m_refreshBtn  = nullptr;
    QPushButton *m_loginBtn    = nullptr;

    int          m_currentWeek = 1;

    QTabWidget  *m_tabs = nullptr;

    QMap<QString, QScrollArea *> m_seriesTabs;
    QMap<QString, DatapackRow *> m_rows;

    SetupManager *m_manager = nullptr;
};
