#include "setupinstaller.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>


SetupInstaller::SetupInstaller(const IracingPathResolver& resolver, QObject* parent)
    : QObject(parent)
    , m_resolver(resolver)
{}


bool SetupInstaller::install(const Setup& setup, const QString& sourceFile)
{
    if (!setup.isValid()) {
        const QString reason = "Setup metadata is invalid (missing car, track, provider, or fileName).";
        qWarning() << "[SetupInstaller]" << reason;
        emit installFailed(setup, reason);
        return false;
    }

    if (!QFile::exists(sourceFile)) {
        const QString reason = QString("Source file does not exist: %1").arg(sourceFile);
        qWarning() << "[SetupInstaller]" << reason;
        emit installFailed(setup, reason);
        return false;
    }

    if (!m_resolver.isIRacingInstalled()) {
        const QString reason = "iRacing setups folder not found. Is iRacing installed?";
        qWarning() << "[SetupInstaller]" << reason;
        emit installFailed(setup, reason);
        return false;
    }

    const QString subFolder = ensureSubFolderExists(setup);
    if (subFolder.isEmpty()) {
        const QString reason = QString("Could not create subfolder for: %1").arg(setup.subFolderName());
        qWarning() << "[SetupInstaller]" << reason;
        emit installFailed(setup, reason);
        return false;
    }

    const QString installPath = resolveInstallPath(setup);
    if (installPath.isEmpty()) {
        const QString reason = "Could not resolve install path.";
        qWarning() << "[SetupInstaller]" << reason;
        emit installFailed(setup, reason);
        return false;
    }

    if (QFile::exists(installPath)) {
        qInfo() << "[SetupInstaller] Overwriting existing setup at:" << installPath;
        QFile::remove(installPath);
    }

    const bool success = QFile::copy(sourceFile, installPath);

    if (success) {
        qInfo() << "[SetupInstaller] Installed:" << installPath;
        emit installed(setup, installPath);
    } else {
        const QString reason = QString("Failed to copy file to: %1").arg(installPath);
        qWarning() << "[SetupInstaller]" << reason;
        emit installFailed(setup, reason);
    }

    return success;
}

bool SetupInstaller::isInstalled(const Setup& setup) const
{
    const QString path = resolveInstallPath(setup);

    if (path.isEmpty()) {
        return false;
    }

    return QFile::exists(path);
}

QString SetupInstaller::resolveInstallPath(const Setup& setup) const
{
    if (!setup.isValid()) {
        return {};
    }
    const QString carPath = m_resolver.setupsPathForCar(setup.car.folderName);
    if (carPath.isEmpty()) {
        return {};
    }
    return QDir::cleanPath(carPath + "/" + setup.subFolderName() + "/" + setup.fileName);
}

bool SetupInstaller::uninstall(const Setup& setup)
{
    const QString path = resolveInstallPath(setup);

    if (path.isEmpty()) {
        qWarning() << "[SetupInstaller] Cannot uninstall — could not resolve path for:" << setup.id;
        return false;
    }
    if (!QFile::exists(path)) {
        qInfo() << "[SetupInstaller] File already gone, nothing to uninstall:" << path;
        return true; // Not an error — file is already not there
    }

    const bool removed = QFile::remove(path);

    if (removed) {
        qInfo() << "[SetupInstaller] Uninstalled:" << path;
        emit uninstalled(setup);
    } else {
        qWarning() << "[SetupInstaller] Failed to remove file:" << path;
    }

    return removed;
}

QString SetupInstaller::ensureSubFolderExists(const Setup& setup) const
{
    const bool carFolderReady = m_resolver.ensureCarFolderExists(setup.car.folderName);
    if (!carFolderReady) {
        return {};
    }
    const QString carPath   = m_resolver.setupsPathForCar(setup.car.folderName);
    const QString subFolder = QDir::cleanPath(carPath + "/" + setup.subFolderName());

    QDir dir(subFolder);

    if (dir.exists()) {
        return subFolder;
    }

    const bool created = dir.mkpath(".");

    if (created) {
        qInfo() << "[SetupInstaller] Created subfolder:" << subFolder;
        return subFolder;
    }

    qWarning() << "[SetupInstaller] Failed to create subfolder:" << subFolder;
    return {};
}
