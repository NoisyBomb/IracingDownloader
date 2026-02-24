#include "gridandgoprovider.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QDateTime>
#include <QDebug>


GridAndGoProvider::GridAndGoProvider(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

void GridAndGoProvider::setAccessToken(const QString& token)
{
    m_token = token;
}

void GridAndGoProvider::fetchDatapackList(int year, int season)
{
    QUrl url(QString("%1/datapacks").arg(BASE_URL));
    QUrlQuery q;
    q.addQueryItem("year",   QString::number(year));
    q.addQueryItem("season", QString::number(season));
    url.setQuery(q);

    QNetworkReply* reply = m_nam->get(makeRequest(url, /*withAuth=*/false));
    connect(reply, &QNetworkReply::finished,
            this,  [this, reply]() { onListReplyFinished(reply); });

    qInfo() << "[GnGProvider] Запрашиваем список датапаков:" << url.toString();
}

void GridAndGoProvider::onListReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        const QString reason = reply->errorString();
        qWarning() << "[GnGProvider] Ошибка загрузки списка:" << reason;
        emit fetchFailed(reason);
        return;
    }

    const QByteArray data = reply->readAll();
    const QList<Setup> setups = parseDatapackList(data);

    qInfo() << "[GnGProvider] Получено датапаков:" << setups.size();
    emit datapackListReady(setups);
}

QList<Setup> GridAndGoProvider::parseDatapackList(const QByteArray& json) const
{
    const QJsonArray arr = QJsonDocument::fromJson(json).array();
    QList<Setup> result;
    result.reserve(arr.size());

    for (const QJsonValue& val : arr) {
        const QJsonObject obj = val.toObject();

        const QString id      = obj["id"].toString();
        const QString carId   = obj["carId"].toString();
        const QString carName = obj["carName"].toString();
        const QString trackName = obj["trackName"].toString();
        const QString author  = obj["author"].toString();
        const QString series  = obj["series"].toString();

        if (id.isEmpty() || carId.isEmpty() || trackName.isEmpty()) {
            continue;
        }

        const Car   car(carId, carName, /*folderName=*/"");
        const Track track(/*id=*/"", trackName);

        Setup s;
        s.id          = id;
        s.displayName = series + " – " + trackName;
        s.author      = author;
        s.provider    = "GnG";
        s.car         = car;
        s.track       = track;

        result.append(s);
    }

    return result;
}


void GridAndGoProvider::fetchDatapackDetails(const QString& datapackId)
{
    if (m_token.isEmpty()) {
        emit fetchFailed("Нет access token — сначала войдите в аккаунт");
        return;
    }

    const QUrl url(QString("%1/datapacks/%2").arg(BASE_URL, datapackId));
    QNetworkReply* reply = m_nam->get(makeRequest(url, /*withAuth=*/true));
    connect(reply, &QNetworkReply::finished,
            this,  [this, reply]() { onDetailsReplyFinished(reply); });

    qInfo() << "[GnGProvider] Запрашиваем детали датапака:" << datapackId;
}

void GridAndGoProvider::onDetailsReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        const QString reason = reply->errorString();
        qWarning() << "[GnGProvider] Ошибка загрузки деталей:" << reason;
        emit fetchFailed(reason);
        return;
    }

    const QByteArray data = reply->readAll();
    const QList<Setup> setups = parseDatapackDetails(data);

    qInfo() << "[GnGProvider] Файлов сетапов в датапаке:" << setups.size();
    emit datapackDetailsReady(setups);
}

QList<Setup> GridAndGoProvider::parseDatapackDetails(const QByteArray& json) const
{
    const QJsonObject obj = QJsonDocument::fromJson(json).object();
    QList<Setup> result;

    const QString datapackId = obj["id"].toString();
    const QString carId      = obj["carId"].toString();
    const QString carName    = obj["carName"].toString();
    const QString trackName  = obj["trackName"].toString();
    const QString trackId    = obj["trackId"].toString();
    const QString author     = obj["author"].toString();
    const QString series     = obj["series"].toString();

    const Car   car(carId, carName, /*folderName=*/"");
    const Track track(trackId, trackName);


    const QJsonArray setupLinks = obj["setupLinks"].toArray();
    for (const QJsonValue& val : setupLinks) {
        const QJsonObject link = val.toObject();
        const QString name = link["name"].toString();
        const QString url  = link["url"].toString();

        if (name.isEmpty() || url.isEmpty()) {
            continue;
        }

        Setup s;
        s.id          = datapackId + "_" + name;
        s.displayName = name;
        s.author      = author;
        s.provider    = "GnG";
        s.car         = car;
        s.track       = track;
        s.downloadUrl = QUrl(url);
        s.fileName    = name;

        result.append(s);
    }

    return result;
}

QNetworkRequest GridAndGoProvider::makeRequest(const QUrl& url, bool withAuth) const
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Origin", "https://app.grid-and-go.com");

    if (withAuth && !m_token.isEmpty()) {
        request.setRawHeader("Authorization",
                             QByteArray("Bearer ") + m_token.toUtf8());
    }

    return request;
}
