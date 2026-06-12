#include "iracingpathresolver.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>


namespace {
constexpr const char* IRACING_FOLDER = "iRacing";
constexpr const char* SETUPS_FOLDER  = "setups";
}


bool IracingPathResolver::isIRacingInstalled() const
{
    const QString path = setupsRootPath();

    if (path.isEmpty()) {
        qWarning() << "[iRacingPathResolver] Could not resolve Documents path.";
        return false;
    }

    const bool exists = QDir(path).exists();

    if (!exists) {
        qWarning() << "[iRacingPathResolver] iRacing setups folder not found at:" << path;
    }
    return exists;
}

QString IracingPathResolver::documentsPath() const
{
    const QStringList locations = QStandardPaths::standardLocations(
        QStandardPaths::DocumentsLocation
        );

    if (locations.isEmpty()) {
        qWarning() << "[iRacingPathResolver] No Documents location found on this system.";
        return {};
    }
    return locations.first();
}

QString IracingPathResolver::iRacingRootPath() const
{
    const QString docs = documentsPath();

    if (docs.isEmpty()) {
        return {};
    }

    return QDir::cleanPath(docs + "/" + IRACING_FOLDER);
}

QString IracingPathResolver::setupsRootPath() const
{
    const QString root = iRacingRootPath();

    if (root.isEmpty()) {
        return {};
    }

    return QDir::cleanPath(root + "/" + SETUPS_FOLDER);
}

QString IracingPathResolver::setupsPathForCar(const QString& carFolderName) const
{
    if (carFolderName.isEmpty()) {
        qWarning() << "[iRacingPathResolver] carFolderName must not be empty.";
        return {};
    }

    const QString root = setupsRootPath();

    if (root.isEmpty()) {
        return {};
    }

    return QDir::cleanPath(root + "/" + carFolderName);
}

bool IracingPathResolver::ensureCarFolderExists(const QString& carFolderName) const
{
    const QString path = setupsPathForCar(carFolderName);

    if (path.isEmpty()) {
        return false;
    }

    QDir dir(path);

    if (dir.exists()) {
        return true; // Already exists — nothing to do
    }

    const bool created = dir.mkpath(".");

    if (created) {
        qInfo() << "[iRacingPathResolver] Created car folder:" << path;
    } else {
        qWarning() << "[iRacingPathResolver] Failed to create car folder:" << path;
    }

    return created;
}
