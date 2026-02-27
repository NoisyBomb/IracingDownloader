#pragma once
#include <QHash>
#include <QString>

class TrackRegistry
{
public:
    static TrackRegistry &instance();
    QString imageFile(const QString &trackName) const;

private:
    TrackRegistry();
    void load();
    QHash<QString, QString> m_tracks; // trackName -> imageFile
};
