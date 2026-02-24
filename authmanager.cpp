#include "authmanager.h"

#include <QWebEngineView>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>


// ── PKCE helpers ──────────────────────────────────────────────────────────────

static QString base64UrlEncode(const QByteArray& data)
{
    return data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

// ── AuthManager ───────────────────────────────────────────────────────────────

AuthManager::AuthManager(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &AuthManager::onTokenReplyFinished);
}

void AuthManager::generatePkce(QString& outVerifier, QString& outChallenge) const
{
    // 32 случайных байта → base64url (43–128 символов, требование PKCE)
    QByteArray raw(32, Qt::Uninitialized);
    QRandomGenerator::global()->fillRange(
        reinterpret_cast<quint32*>(raw.data()),
        raw.size() / sizeof(quint32));

    outVerifier = base64UrlEncode(raw);

    // challenge = BASE64URL(SHA-256(verifier))
    const QByteArray hash = QCryptographicHash::hash(
        outVerifier.toUtf8(), QCryptographicHash::Sha256);
    outChallenge = base64UrlEncode(hash);
}

void AuthManager::startLogin()
{
    // Генерируем PKCE-пару
    QString codeChallenge;
    generatePkce(m_codeVerifier, codeChallenge);

    // Строим URL авторизации с code_challenge
    QUrl url(AUTH_URL);
    QUrlQuery q(url);
    q.addQueryItem("code_challenge",        codeChallenge);
    q.addQueryItem("code_challenge_method", "S256");
    url.setQuery(q);

    // Создаём WebView-окно (показываем пользователю)
    if (!m_webView) {
        m_webView = new QWebEngineView();
        m_webView->setWindowTitle("Войти в Grid-and-Go");
        m_webView->resize(520, 680);

        connect(m_webView, &QWebEngineView::urlChanged,
                this,       &AuthManager::onUrlChanged);
    }

    m_webView->load(url);
    m_webView->show();

    qInfo() << "[AuthManager] Открываем страницу авторизации:" << url.toString();
}

void AuthManager::onUrlChanged(const QUrl& url)
{
    const QString urlStr = url.toString();

    // Ждём редирект на redirect_uri с параметром ?code=
    if (!urlStr.startsWith(REDIRECT_URI)) {
        return;
    }

    const QUrlQuery query(url);
    const QString code = query.queryItemValue("code");

    if (code.isEmpty()) {
        const QString error = query.queryItemValue("error_description");
        qWarning() << "[AuthManager] Редирект без code. error:" << error;
        emit loginFailed(error.isEmpty() ? "Авторизация отменена" : error);
        m_webView->hide();
        return;
    }

    qInfo() << "[AuthManager] Получили code, обмениваем на токен…";
    m_webView->hide();

    exchangeCodeForToken(code, m_codeVerifier);
}

void AuthManager::exchangeCodeForToken(const QString& code,
                                       const QString& codeVerifier)
{
    // Явно создаём QUrl — избегаем Most Vexing Parse
    const QUrl tokenUrl = QUrl(QString(TOKEN_URL));
    QNetworkRequest request;
    request.setUrl(tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QByteArray("application/x-www-form-urlencoded"));

    QUrlQuery body;
    body.addQueryItem("grant_type",    "authorization_code");
    body.addQueryItem("client_id",     QString(CLIENT_ID));
    body.addQueryItem("redirect_uri",  QString(REDIRECT_URI));
    body.addQueryItem("code_verifier", codeVerifier);
    body.addQueryItem("code",          code);

    const QByteArray postData = body.toString(QUrl::FullyEncoded).toUtf8();
    m_nam->post(request, postData);
}

void AuthManager::onTokenReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        const QString reason = reply->errorString();
        qWarning() << "[AuthManager] Ошибка обмена токена:" << reason;
        emit loginFailed(reason);
        return;
    }

    const QByteArray    raw  = reply->readAll();
    const QJsonDocument doc  = QJsonDocument::fromJson(raw);
    const QJsonObject   json = doc.object();

    if (json.contains("error")) {
        const QString reason = json["error_description"].toString(json["error"].toString());
        qWarning() << "[AuthManager] Сервер вернул ошибку:" << reason;
        emit loginFailed(reason);
        return;
    }

    m_accessToken  = json["access_token"].toString();
    m_refreshToken = json["refresh_token"].toString();

    if (m_accessToken.isEmpty()) {
        emit loginFailed("access_token отсутствует в ответе сервера");
        return;
    }

    qInfo() << "[AuthManager] Авторизация успешна.";
    emit loginSucceeded(m_accessToken);
}

void AuthManager::logout()
{
    m_accessToken.clear();
    m_refreshToken.clear();
    m_codeVerifier.clear();
    emit loggedOut();
}
