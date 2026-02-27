#pragma once

#include <QString>


struct Car
{
    QString id;
    QString displayName;
    QString folderName;

    Car() = default;
    Car(const QString& id,
        const QString& displayName,
        const QString& folderName)
        : id(id)
        , displayName(displayName)
        , folderName(folderName)
    {}

    bool isValid() const { return !folderName.isEmpty(); }

    bool operator==(const Car& other) const { return id == other.id; }
    bool operator!=(const Car& other) const { return !(*this == other); }
};
