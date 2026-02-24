#pragma once

#include "car.h"
#include "track.h"

#include <QString>
#include <QUrl>
#include <QDateTime>

struct Setup
{
    // Unique id of this individual .sto file
    QString  id;

    // Id of the parent datapack (one datapack may have multiple .sto files)
    QString  datapackId;

    QString  displayName;
    QString  author;
    QString  provider;
    QString  series;

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
          const QString& provider   = {},
          const QString& author     = {},
          const QString& series     = {},
          const QString& datapackId = {})
        : id(id)
        , datapackId(datapackId)
        , displayName(displayName)
        , author(author)
        , provider(provider)
        , series(series)
        , car(car)
        , track(track)
        , downloadUrl(downloadUrl)
        , fileName(fileName)
    {}

    QString subFolderName() const
    {
        if (provider.isEmpty())
            return track.displayName;
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
