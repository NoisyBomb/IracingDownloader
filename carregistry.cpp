#include "carregistry.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QRegularExpression>

CarRegistry &CarRegistry::instance()
{
    static CarRegistry s_instance;
    return s_instance;
}

CarRegistry::CarRegistry()
{
    load();
}

void CarRegistry::load()
{
    QFile file(":/data/cars.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "CarRegistry: could not open :/data/cars.json";
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonArray cars = doc.object().value("cars").toArray();

    for (const QJsonValue &val : cars) {
        const QJsonObject obj = val.toObject();
        CarInfo info;
        info.carId       = obj.value("carId").toString();
        info.displayName = obj.value("displayName").toString();
        info.folderName  = obj.value("folderName").toString();
        m_cars.insert(info.carId, info);
    }

    qDebug() << "CarRegistry: loaded" << m_cars.size() << "cars";
}

QString CarRegistry::folderName(const QString &carId, const QString &fallbackDisplayName) const
{
    if (m_cars.contains(carId))
        return m_cars[carId].folderName;

    qWarning() << "CarRegistry: unknown carId" << carId << "- using fallback";

    QString name = fallbackDisplayName.isEmpty() ? carId : fallbackDisplayName;
    name = name.toLower();
    name.replace(' ', '_');
    static const QRegularExpression s_invalidChars("[^a-z0-9_]");
    name.remove(s_invalidChars);
    return name;
}

CarInfo CarRegistry::carInfo(const QString &carId) const
{
    return m_cars.value(carId);
}

bool CarRegistry::isKnown(const QString &carId) const
{
    return m_cars.contains(carId);
}
