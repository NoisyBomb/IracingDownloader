#include "authmanager.h"

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QTimer>
#include <QDebug>

static QString base64UrlEncode(const QByteArray& data)
{
    return data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

AuthManager::AuthManager(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

void AuthManager::generatePkce(QString& outVerifier, QString& outChallenge) const
{
    QByteArray raw(32, Qt::Uninitialized);
    QRandomGenerator::global()->fillRange(
        reinterpret_cast<quint32*>(raw.data()),
        raw.size() / sizeof(quint32));

    outVerifier  = base64UrlEncode(raw);
    const QByteArray hash = QCryptographicHash::hash(
        outVerifier.toUtf8(), QCryptographicHash::Sha256);
    outChallenge = base64UrlEncode(hash);
}

void AuthManager::startLogin()
{
    QString codeChallenge;
    generatePkce(m_codeVerifier, codeChallenge);

    QUrl url(AUTH_URL);
    QUrlQuery q(url);
    q.addQueryItem("code_challenge",        codeChallenge);
    q.addQueryItem("code_challenge_method", "S256");
    url.setQuery(q);

    if (!m_webView) {
        m_webView = new QWebEngineView();
        m_webView->setWindowTitle("Grid-and-Go — Login");
        m_webView->resize(520, 680);

        connect(m_webView, &QWebEngineView::urlChanged,
                this,       &AuthManager::onUrlChanged);

        // AUTO-LOGIN DISABLED
        // connect(m_webView, &QWebEngineView::loadFinished,
        //         this, &AuthManager::onPageLoaded);
    }

    m_webView->load(url);
    m_webView->show();
    qInfo() << "[AuthManager] Opening login page…";
}

void AuthManager::onPageLoaded(bool ok)
{
    if (!ok) return;

    const QString url = m_webView->url().toString();

    // Только на странице логина Cognito
    if (!url.contains("amazoncognito.com/login"))
        return;

    // JS: заполняем поля и сабмитим форму
    const QString js = QString(R"(
        (function() {
            var emailField    = document.getElementById('signInFormUsername');
            var passwordField = document.getElementById('signInFormPassword');
            var submitBtn     = document.querySelector('input[name="signInSubmitButton"]');

            if (!emailField || !passwordField || !submitBtn) return 'fields_not_found';

            // Устанавливаем значения через нативный setter чтобы React/Vue их увидел
            var nativeInputValueSetter = Object.getOwnPropertyDescriptor(
                window.HTMLInputElement.prototype, 'value').set;
            nativeInputValueSetter.call(emailField,    '%1');
            nativeInputValueSetter.call(passwordField, '%2');

            emailField.dispatchEvent(new Event('input', { bubbles: true }));
            passwordField.dispatchEvent(new Event('input', { bubbles: true }));

            submitBtn.click();
            return 'submitted';
        })()
    )").arg(QString(HARDCODED_USERNAME), QString(HARDCODED_PASSWORD));

    // Небольшая задержка чтобы страница полностью отрисовалась
    QTimer::singleShot(800, this, [this, js]() {
        m_webView->page()->runJavaScript(js, [](const QVariant &result) {
            qInfo() << "[AuthManager] Autofill result:" << result.toString();
        });
    });
}

void AuthManager::onUrlChanged(const QUrl& url)
{
    const QString urlStr = url.toString();

    if (!urlStr.startsWith(REDIRECT_URI))
        return;

    const QUrlQuery query(url);
    const QString code = query.queryItemValue("code");

    if (code.isEmpty()) {
        const QString error = query.queryItemValue("error_description");
        emit loginFailed(error.isEmpty() ? "Login cancelled" : error);
        m_webView->hide();
        return;
    }

    qInfo() << "[AuthManager] Got code, exchanging for token…";
    m_webView->hide();
    exchangeCodeForToken(code, m_codeVerifier);
}

void AuthManager::exchangeCodeForToken(const QString& code, const QString& codeVerifier)
{
    QNetworkRequest request;
    request.setUrl(QUrl(TOKEN_URL));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QByteArray("application/x-www-form-urlencoded"));

    QUrlQuery body;
    body.addQueryItem("grant_type",    "authorization_code");
    body.addQueryItem("client_id",     QString(CLIENT_ID));
    body.addQueryItem("redirect_uri",  QString(REDIRECT_URI));
    body.addQueryItem("code_verifier", codeVerifier);
    body.addQueryItem("code",          code);

    QNetworkReply* reply = m_nam->post(request, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished,
            this,  [this, reply]() { onTokenReplyFinished(reply); });
}

void AuthManager::onTokenReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit loginFailed(reply->errorString());
        return;
    }

    const QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();

    if (json.contains("error")) {
        emit loginFailed(json["error_description"].toString(json["error"].toString()));
        return;
    }

    m_accessToken  = json["access_token"].toString();
    m_refreshToken = json["refresh_token"].toString();

    if (m_accessToken.isEmpty()) {
        emit loginFailed("access_token missing in response");
        return;
    }

    qInfo() << "[AuthManager] Login successful.";
    emit loginSucceeded(m_accessToken);
}

void AuthManager::logout()
{
    m_accessToken.clear();
    m_refreshToken.clear();
    m_codeVerifier.clear();
    emit loggedOut();
}
