#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include "data.h"
#include <QSettings>
#include <iostream>
#include <QDebug>
#include "settingsdb.h"
#include "city.h"
#include "connection.h"
#include "overlay.h"

class Configuration;
class SQL;



struct query
{
 bool b_stop_query_on_error = false;
 bool b_sql_execute_after_loading = false;
 QString s_query_path;
 QByteArray geometry;
};

class Configuration : public QObject
{
 Q_OBJECT
public:
 City* currCity;
 Connection* currConnection;
 void getSettings();
 void saveSettings();
 void setOverlay(Overlay* ov);
 QMap<QString, City*> cityList;
 QMap<QString, Overlay*>* overlayMap = new QMap<QString, Overlay*>();
 qint32 currentCityId;
 query q;
 static Configuration* instance();
 bool bDisplayWebDebug = false;
 bool bRunInBrowser = false;
 QStringList localOverlayList;
 QStringList georeferencedList;
 QString path;
 QStringList cityNames();
 QString lookupCityName(Bounds b);
 QMap<QString, Bounds> cityBounds;
private:
 static Configuration* _instance;
 explicit Configuration(QObject *parent = 0);

signals:

public slots:

};

#endif // CONFIGURATION_H
