#include "trackregistry.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QDebug>

TrackRegistry &TrackRegistry::instance()
{
    static TrackRegistry s;
    return s;
}

TrackRegistry::TrackRegistry() { load(); }

void TrackRegistry::load()
{
    QFile file(":/data/tracks.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "TrackRegistry: cannot open :/data/tracks.json";
        return;
    }
    const QJsonArray arr = QJsonDocument::fromJson(file.readAll())
                               .object().value("tracks").toArray();
    for (const QJsonValue &v : arr) {
        const QJsonObject o = v.toObject();
        m_tracks.insert(o["trackName"].toString(), o["imageFile"].toString());
    }
    qDebug() << "TrackRegistry: loaded" << m_tracks.size() << "tracks";
}

QString TrackRegistry::imageFile(const QString &trackName) const
{
    if (m_tracks.contains(trackName))
        return m_tracks[trackName];

    // Fallback: sanitize
    static const QRegularExpression re("[^a-z0-9_]");
    QString s = trackName.toLower();
    s.replace(' ', '_');
    s.remove(re);
    qWarning() << "TrackRegistry: unknown track" << trackName << "-> fallback:" << s;
    return s;
}
