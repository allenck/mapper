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

    /*virtual*/ ~FileDownloader();

    QByteArray downloadedData() const;
    void setOverlay(Overlay* ov);
    Overlay* overlay();
    QString error();
signals:
    void downloaded(QString error);

private slots:

    void fileDownloaded(QNetworkReply* pReply);

private:

    QNetworkAccessManager* m_WebCtrl = nullptr;

    QByteArray m_DownloadedData;
    Overlay* ov = nullptr;
    QUrl imageUrl;
    QString errorString;
};

#endif // FILEDOWNLOADER_H

