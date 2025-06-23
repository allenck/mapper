#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QUrl>
#include <QWidget>

class MapView : public QWidget
{
    Q_OBJECT

public:
    MapView(QWidget *parent = 0);
    void setBaseUrl(const QUrl &url);

//public slots:
//    void on_loadHtml();

private:
    QUrl baseUrl;


};

#endif // MAPVIEW_H
