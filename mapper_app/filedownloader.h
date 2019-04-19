#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class Overlay;
class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QUrl imageUrl, QObject *parent = 0);

    virtual ~FileDownloader();

    QByteArray downloadedData() const;
    void setOverlay(Overlay* ov);
    Overlay* overlay();

signals:
        void downloaded();

private slots:

    void fileDownloaded(QNetworkReply* pReply);

private:

    QNetworkAccessManager m_WebCtrl;

    QByteArray m_DownloadedData;
    Overlay* ov = nullptr;
};

#endif // FILEDOWNLOADER_H

