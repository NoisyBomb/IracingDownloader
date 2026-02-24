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

    // ── Авторизация ───────────────────────────────────
    void login();
    void logout();
    bool isLoggedIn() const;

    // ── Данные ────────────────────────────────────────
    void refreshDatapackList(int year = 2026, int season = 1);
    const QList<Setup>& setups() const { return m_setups; }

    // Загрузить детали датапака (setupLinks)
    void loadDatapackDetails(const QString& datapackId);

    // Скачать и установить конкретный сетап
    void downloadAndInstall(const Setup& setup);

    // Удалить / проверить
    bool uninstall(const Setup& setup);
    bool isInstalled(const Setup& setup) const;

signals:
    // Авторизация
    void loginSucceeded();
    void loginFailed(const QString& reason);
    void loggedOut();

    // Список
    void setupListUpdated(const QList<Setup>& setups);

    // Детали датапака (datapackId нужен MainWindow чтобы найти нужную карточку)
    void datapackDetailsLoaded(const QString& datapackId, const QList<Setup>& setups);

    // Загрузка и установка
    void downloadProgress(const Setup& setup, qint64 received, qint64 total);
    void installSucceeded(const Setup& setup, const QString& path);
    void installFailed(const Setup& setup, const QString& reason);

    // Общие ошибки
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
