#include "filedownloader.h"
#include <QDebug>

// http://developer.qt.nokia.com/wiki/Download_Data_from_URL

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
    QObject(parent)
{
    connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
                SLOT(fileDownloaded(QNetworkReply*)));

    QNetworkRequest request(imageUrl);
    m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader()
{

}

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
    m_DownloadedData = pReply->readAll();
    if(pReply->error() > 0)
        qDebug() << pReply->errorString();
    //emit a signal
    emit downloaded();
}

QByteArray FileDownloader::downloadedData() const
{
    return m_DownloadedData;
}
