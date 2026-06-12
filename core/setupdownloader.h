#pragma once

#include "models/setup.h"

#include <QObject>
#include <QString>
#include <QNetworkRequest>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

class SetupDownloader : public QObject
{
    Q_OBJECT

public:
    explicit SetupDownloader(QObject* parent = nullptr);
    void download(const Setup& setup);
    void cancel();

signals:
    void downloadFinished(const Setup& setup, const QString& tempFilePath);
    void downloadFailed(const Setup& setup, const QString& reason);
    void downloadProgress(const Setup& setup, qint64 received, qint64 total);

private slots:
    void onReadyRead();
    void onFinished();
    void onProgress(qint64 received, qint64 total);

private:
    void cleanup();

    QNetworkAccessManager* m_nam     = nullptr;
    QNetworkReply*         m_reply   = nullptr;
    QFile*                 m_file    = nullptr;
    Setup                  m_current;
    QString                m_tempPath;
};
