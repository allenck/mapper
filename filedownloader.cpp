#include "filedownloader.h"
#include <QDebug>

// http://developer.qt.nokia.com/wiki/Download_Data_from_URL

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
    QObject(parent)
{
 qDebug() << "download " << imageUrl.toDisplayString();
    connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)), SLOT(fileDownloaded(QNetworkReply*)));
    this->imageUrl = imageUrl;
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
    {
        qDebug() << "download failed " << imageUrl.toDisplayString() << pReply->errorString();
        errorString =pReply->errorString();
    }
    //emit a signal
    emit downloaded(pReply->error()==0?"":pReply->errorString());
}

QByteArray FileDownloader::downloadedData() const
{
    return m_DownloadedData;
}

void FileDownloader::setOverlay(Overlay* ov) {this->ov = ov;}
Overlay* FileDownloader::overlay() {return ov;}
QString FileDownloader::error() {return errorString;}
