#pragma once

#include <QString>
#include <QDir>

class IracingPathResolver
{
public:
    IracingPathResolver() = default;

    bool isIRacingInstalled() const;
    QString documentsPath() const;
    QString setupsRootPath() const;
    QString setupsPathForCar(const QString& carFolderName) const;
    bool ensureCarFolderExists(const QString& carFolderName) const;
    QString iRacingRootPath() const;
};
