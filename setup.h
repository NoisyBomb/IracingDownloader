#pragma once

#include "car.h"
#include "track.h"

#include <QString>
#include <QUrl>
#include <QDateTime>

struct Setup
{
    QString  id;
    QString  displayName;
    QString  author;
    QString  provider;

    Car      car;
    Track    track;

    QUrl     downloadUrl;
    QString  fileName;
    QDateTime uploadedAt;
    bool isInstalled = false;

    Setup() = default;
    Setup(const QString& id,
          const QString& displayName,
          const Car&     car,
          const Track&   track,
          const QUrl&    downloadUrl,
          const QString& fileName,
          const QString& provider = {},
          const QString& author   = {})
        : id(id)
        , displayName(displayName)
        , author(author)
        , provider(provider)
        , car(car)
        , track(track)
        , downloadUrl(downloadUrl)
        , fileName(fileName)
    {}

    QString subFolderName() const
    {
        if (provider.isEmpty()) {
            return track.displayName;
        }
        return provider + "_" + track.displayName;
    }

    bool isValid() const
    {
        return !id.isEmpty()
        && car.isValid()
            && track.isValid()
            && downloadUrl.isValid()
            && !fileName.isEmpty()
            && !provider.isEmpty();
    }

    bool operator==(const Setup& other) const { return id == other.id; }
    bool operator!=(const Setup& other) const { return !(*this == other); }
};
