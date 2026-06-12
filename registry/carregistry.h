#pragma once

#include <QHash>
#include <QString>

struct CarInfo {
    QString carId;
    QString displayName;
    QString folderName;
};

class CarRegistry
{
public:
    static CarRegistry &instance();
    QString folderName(const QString &carId, const QString &fallbackDisplayName = {}) const;

    CarInfo carInfo(const QString &carId) const;

    bool isKnown(const QString &carId) const;

private:
    CarRegistry();
    void load();

    QHash<QString, CarInfo> m_cars;
};
