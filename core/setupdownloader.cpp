#include "setupdownloader.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>


SetupDownloader::SetupDownloader(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

void SetupDownloader::download(const Setup& setup)
{
    if (!setup.downloadUrl.isValid() || setup.downloadUrl.isEmpty()) {
        emit downloadFailed(setup, "Некорректный URL для скачивания");
        return;
    }

    if (m_reply) {
        emit downloadFailed(setup, "Уже идёт загрузка, дождитесь окончания");
        return;
    }

    m_current = setup;
    const QString tempDir = QStandardPaths::writableLocation(
                                QStandardPaths::TempLocation) + "/IracingDownloader";
    QDir().mkpath(tempDir);
    m_tempPath = tempDir + "/" + setup.fileName;

    m_file = new QFile(m_tempPath, this);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString reason = QString("Не удалось создать временный файл: %1").arg(m_tempPath);
        qWarning() << "[SetupDownloader]" << reason;
        emit downloadFailed(setup, reason);
        cleanup();
        return;
    }

    QNetworkRequest request;
    request.setUrl(setup.downloadUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_nam->get(request);

    connect(m_reply, &QNetworkReply::readyRead,
            this,    &SetupDownloader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished,
            this,    &SetupDownloader::onFinished);
    connect(m_reply, &QNetworkReply::downloadProgress,
            this,    &SetupDownloader::onProgress);

    qInfo() << "[SetupDownloader] Начинаем скачивание:" << setup.fileName;
}

void SetupDownloader::onReadyRead()
{
    if (m_file && m_reply) {
        m_file->write(m_reply->readAll());
    }
}

void SetupDownloader::onProgress(qint64 received, qint64 total)
{
    emit downloadProgress(m_current, received, total);
}

void SetupDownloader::onFinished()
{
    if (!m_reply) {
        return;
    }
    const QNetworkReply::NetworkError error = m_reply->error();
    if (error != QNetworkReply::NoError) {
        const int httpStatus = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString reason = m_reply->errorString();
        if (httpStatus == 403) {
            reason = "Ссылка для скачивания истекла (10 мин). Обновите список сетапов.";
        }
        qWarning() << "[SetupDownloader] Ошибка:" << reason;
        emit downloadFailed(m_current, reason);
        cleanup();
        return;
    }
    if (m_file && m_reply->bytesAvailable() > 0) {
        m_file->write(m_reply->readAll());
    }
    if (!m_file) {
        emit downloadFailed(m_current, "Временный файл был закрыт во время загрузки");
        cleanup();
        return;
    }

    m_file->close();
    const QString savedPath = m_tempPath;
    const Setup   savedSetup = m_current;
    qInfo() << "[SetupDownloader] Скачано:" << savedPath;
    cleanup();
    emit downloadFinished(savedSetup, savedPath);
}

void SetupDownloader::cancel()
{
    if (m_reply) {
        m_reply->abort();
        qInfo() << "[SetupDownloader] Загрузка отменена";
    }
    cleanup();
}

void SetupDownloader::cleanup()
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_file) {
        m_file->deleteLater();
        m_file = nullptr;
    }
    m_tempPath.clear();
    m_current = {};
}
