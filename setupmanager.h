#pragma once

#include "setup.h"

#include <QObject>
#include <QString>
#include <QList>

class AuthManager;
class GridAndGoProvider;
class SetupDownloader;
class SetupInstaller;
class IracingPathResolver;

class SetupManager : public QObject
{
    Q_OBJECT

public:
    explicit SetupManager(QObject* parent = nullptr);

    void login();
    void logout();
    bool isLoggedIn() const;

    void refreshDatapackList(int year = 0, int season = 0);
    const QList<Setup>& setups() const { return m_setups; }

    void loadDatapackDetails(const QString& datapackId);

    void downloadAndInstall(const Setup& setup);

    bool uninstall(const Setup& setup);
    bool isInstalled(const Setup& setup) const;

signals:
    void loginSucceeded();
    void loginFailed(const QString& reason);
    void loggedOut();

    void setupListUpdated(const QList<Setup>& setups);

    void datapackDetailsLoaded(const QString& datapackId, const QList<Setup>& setups);

    void downloadProgress(const Setup& setup, qint64 received, qint64 total);
    void installSucceeded(const Setup& setup, const QString& path);
    void installFailed(const Setup& setup, const QString& reason);

    void errorOccurred(const QString& reason);

private slots:
    void onLoginSucceeded(const QString& token);
    void onLoginFailed(const QString& reason);

    void onDatapackListReady(const QList<Setup>& setups);
    void onDatapackDetailsReady(const QString& datapackId, const QList<Setup>& setups);

    void onDownloadFinished(const Setup& setup, const QString& tempFilePath);
    void onDownloadFailed(const Setup& setup, const QString& reason);
    void onDownloadProgress(const Setup& setup, qint64 received, qint64 total);

private:
    IracingPathResolver*  m_resolver   = nullptr;
    AuthManager*          m_auth       = nullptr;
    GridAndGoProvider*    m_provider   = nullptr;
    SetupDownloader*      m_downloader = nullptr;
    SetupInstaller*       m_installer  = nullptr;

    QList<Setup> m_setups;
};
