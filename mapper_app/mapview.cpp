#include <QtGui>
#include "mapview.h"

MapView::MapView(QWidget *parent) :
    QWidget(parent)
{
    //setupUi(this);
}
void MapView::setBaseUrl(const QUrl &url)
{
    baseUrl = url;
}

//void on_loadHtml(QString text)
//{
//    // update the contents in web_viewer
//    //MainWindow::centralWidget->webView->setHtml(text, baseUrl);
//}

