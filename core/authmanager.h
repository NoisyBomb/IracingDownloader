#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class QWebEngineView;
class QNetworkAccessManager;
class QNetworkReply;

class AuthManager : public QObject
{
    Q_OBJECT

public:
    explicit AuthManager(QObject* parent = nullptr);

    void startLogin();

    QString accessToken()  const { return m_accessToken; }
    QString refreshToken() const { return m_refreshToken; }
    bool    isLoggedIn()   const { return !m_accessToken.isEmpty(); }
    bool m_tokenExchanged = false;

signals:
    void loginSucceeded(const QString& accessToken);
    void loginFailed(const QString& reason);
    void loggedOut();

public slots:
    void logout();

private slots:
    void onUrlChanged(const QUrl& url);
    void onPageLoaded(bool ok);
    void onTokenReplyFinished(QNetworkReply* reply);

private:
    void exchangeCodeForToken(const QString& code, const QString& codeVerifier);
    void generatePkce(QString& outVerifier, QString& outChallenge) const;

    QWebEngineView*        m_webView   = nullptr;
    QNetworkAccessManager* m_nam       = nullptr;

    QString m_codeVerifier;
    QString m_accessToken;
    QString m_refreshToken;

    static constexpr const char* CLIENT_ID    = "1nqqluo9th1iajur09j2amd63p";
    static constexpr const char* REDIRECT_URI = "https://app.grid-and-go.com";
    static constexpr const char* TOKEN_URL    =
        "https://grid-and-go-auth.auth.eu-central-1.amazoncognito.com/oauth2/token";
    static constexpr const char* AUTH_URL     =
        "https://grid-and-go-auth.auth.eu-central-1.amazoncognito.com/login"
        "?response_type=code"
        "&client_id=1nqqluo9th1iajur09j2amd63p"
        "&redirect_uri=https://app.grid-and-go.com";

    static constexpr const char* HARDCODED_USERNAME = "******";
    static constexpr const char* HARDCODED_PASSWORD = "******";
};
