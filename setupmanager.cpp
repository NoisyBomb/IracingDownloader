#include "setupmanager.h"
#include "iracingweek.h"

#include "authmanager.h"
#include "gridandgoprovider.h"
#include "setupdownloader.h"
#include "setupinstaller.h"
#include "iracingpathresolver.h"

#include <QDebug>


SetupManager::SetupManager(QObject* parent)
    : QObject(parent)
    , m_resolver  (new IracingPathResolver())
    , m_auth      (new AuthManager(this))
    , m_provider  (new GridAndGoProvider(this))
    , m_downloader(new SetupDownloader(this))
    , m_installer (new SetupInstaller(*m_resolver, this))
{
    connect(m_auth, &AuthManager::loginSucceeded,
            this,   &SetupManager::onLoginSucceeded);
    connect(m_auth, &AuthManager::loginFailed,
            this,   &SetupManager::onLoginFailed);
    connect(m_auth, &AuthManager::loggedOut,
            this,   &SetupManager::loggedOut);

    connect(m_provider, &GridAndGoProvider::datapackListReady,
            this,        &SetupManager::onDatapackListReady);
    connect(m_provider, &GridAndGoProvider::datapackDetailsReady,
            this,        &SetupManager::onDatapackDetailsReady);
    connect(m_provider, &GridAndGoProvider::fetchFailed,
            this,        &SetupManager::errorOccurred);

    connect(m_downloader, &SetupDownloader::downloadFinished,
            this,          &SetupManager::onDownloadFinished);
    connect(m_downloader, &SetupDownloader::downloadFailed,
            this,          &SetupManager::onDownloadFailed);
    connect(m_downloader, &SetupDownloader::downloadProgress,
            this,          &SetupManager::onDownloadProgress);

    connect(m_installer, &SetupInstaller::installed,
            this,         &SetupManager::installSucceeded);
    connect(m_installer, &SetupInstaller::installFailed,
            this,         &SetupManager::installFailed);
}

void SetupManager::login()
{
    m_auth->startLogin();
}

void SetupManager::logout()
{
    m_auth->logout();
    m_setups.clear();
}

bool SetupManager::isLoggedIn() const
{
    return m_auth->isLoggedIn();
}

void SetupManager::onLoginSucceeded(const QString& token)
{
    m_provider->setAccessToken(token);
    qInfo() << "[SetupManager] Авторизация успешна, загружаем список датапаков";
    emit loginSucceeded();
    refreshDatapackList();
}

void SetupManager::onLoginFailed(const QString& reason)
{
    qWarning() << "[SetupManager] Ошибка авторизации:" << reason;
    emit loginFailed(reason);
}

void SetupManager::refreshDatapackList(int year, int season)
{
    const IracingWeek cur = IracingWeek::current();
    const int y = (year   > 0) ? year   : cur.year;
    const int s = (season > 0) ? season : cur.season;
    m_provider->fetchDatapackList(y, s);
}

void SetupManager::onDatapackListReady(const QList<Setup>& setups)
{
    m_setups = setups;
    qInfo() << "[SetupManager] Список обновлён, датапаков:" << m_setups.size();
    emit setupListUpdated(m_setups);
}

void SetupManager::loadDatapackDetails(const QString& datapackId)
{
    m_provider->fetchDatapackDetails(datapackId);
}

void SetupManager::onDatapackDetailsReady(const QString& datapackId, const QList<Setup>& setups)
{
    qInfo() << "[SetupManager] Детали датапака загружены, файлов:" << setups.size();
    emit datapackDetailsLoaded(datapackId, setups);
}

void SetupManager::downloadAndInstall(const Setup& setup)
{
    qInfo() << "[SetupManager] Скачиваем сетап:" << setup.fileName;
    m_downloader->download(setup);
}

void SetupManager::onDownloadFinished(const Setup& setup, const QString& tempFilePath)
{
    qInfo() << "[SetupManager] Скачан, устанавливаем:" << setup.fileName;
    m_installer->install(setup, tempFilePath);
}

void SetupManager::onDownloadFailed(const Setup& setup, const QString& reason)
{
    qWarning() << "[SetupManager] Ошибка скачивания:" << reason;
    emit installFailed(setup, reason);
}

void SetupManager::onDownloadProgress(const Setup& setup, qint64 received, qint64 total)
{
    emit downloadProgress(setup, received, total);
}

bool SetupManager::uninstall(const Setup& setup)
{
    return m_installer->uninstall(setup);
}

bool SetupManager::isInstalled(const Setup& setup) const
{
    return m_installer->isInstalled(setup);
}
