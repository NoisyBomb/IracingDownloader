#pragma once

#include <QString>


struct Track
{
    QString id;
    QString displayName;
    QString config;


    Track() = default;
    Track(const QString& id,
          const QString& displayName,
          const QString& config = {})
        : id(id)
        , displayName(displayName)
        , config(config)
    {}


    QString fullName() const
    {
        if (config.isEmpty()) {
            return displayName;
        }
        return displayName + " \u2013 " + config;
    }

    bool isValid() const { return !id.isEmpty() && !displayName.isEmpty(); }

    bool operator==(const Track& other) const { return id == other.id && config == other.config; }
    bool operator!=(const Track& other) const { return !(*this == other); }
};
