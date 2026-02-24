#pragma once

#include "setup.h"

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

    // Шаг 1: загрузить список датапаков (без токена)
    void fetchDatapackList(int year = 2026, int season = 1);

    // Шаг 2: загрузить детали одного датапака (с токеном) → получаем setupLinks
    void fetchDatapackDetails(const QString& datapackId);

signals:
    // Список датапаков загружен (базовая инфа, без setupLinks)
    void datapackListReady(const QList<Setup>& setups);

    // Детали готовы: datapackId + список Setup (по одному на каждый .sto файл)
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
