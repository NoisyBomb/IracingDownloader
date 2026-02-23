#pragma once

#include "setup.h"
#include "iracingpathresolver.h"

#include <QString>
#include <QObject>

class SetupInstaller : public QObject
{
    Q_OBJECT

public:
    explicit SetupInstaller(const IracingPathResolver& resolver, QObject* parent = nullptr);

    bool install(const Setup& setup, const QString& sourceFile);
    bool isInstalled(const Setup& setup) const;

    QString resolveInstallPath(const Setup& setup) const;

    bool uninstall(const Setup& setup);

signals:
    void installed(const Setup& setup, const QString& installPath);

    void installFailed(const Setup& setup, const QString& reason);
    void uninstalled(const Setup& setup);

private:
    QString ensureSubFolderExists(const Setup& setup) const;

    const IracingPathResolver& m_resolver;
};
