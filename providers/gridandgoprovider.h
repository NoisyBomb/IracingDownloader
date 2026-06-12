#pragma once

#include "models/setup.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QNetworkRequest>
#include <utility>

class QNetworkAccessManager;
class QNetworkReply;

class GridAndGoProvider : public QObject
{
    Q_OBJECT

public:
    explicit GridAndGoProvider(QObject* parent = nullptr);

    void setAccessToken(const QString& token);

    void fetchDatapackList(int year = 2026, int season = 1);

    void fetchDatapackDetails(const QString& datapackId);

signals:
    void datapackListReady(const QList<Setup>& setups);

    void datapackDetailsReady(const QString& datapackId, const QList<Setup>& setups);

    void fetchFailed(const QString& reason);

private slots:
    void onListReplyFinished(QNetworkReply* reply);
    void onDetailsReplyFinished(QNetworkReply* reply);

private:
    QNetworkRequest makeRequest(const QUrl& url, bool withAuth = false) const;

    QList<Setup> parseDatapackList(const QByteArray& json) const;
    std::pair<QString, QList<Setup>> parseDatapackDetails(const QByteArray& json) const;

    QNetworkAccessManager* m_nam   = nullptr;
    QString                m_token;

    static constexpr const char* BASE_URL =
        "https://oaseb2ya72.execute-api.eu-central-1.amazonaws.com";
};
