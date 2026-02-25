#pragma once
#include <QHash>
#include <QString>

class TrackRegistry
{
public:
    static TrackRegistry &instance();
    // Returns image filename (without extension) for given trackName
    // Falls back to sanitized trackName if not found
    QString imageFile(const QString &trackName) const;

private:
    TrackRegistry();
    void load();
    QHash<QString, QString> m_tracks; // trackName -> imageFile
};
