#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include "data.h"
#include <QSettings>
//#include <iostream>
#include <QDebug>
#include "city.h"
#include "connection.h"
#include "qapplication.h"
#include <QFont>

class Configuration;
class SQL;



struct query
{
 bool b_stop_query_on_error = false;
 bool b_sql_execute_after_loading = false;
 QString s_query_path;
 QByteArray geometry;
};

struct routeView
{
  QList<QVariant> hiddenColumns;
  QList<QVariant> movedColumns;
  //QByteArray state;
};

class Configuration : public QObject
{
 Q_OBJECT
public:
 City* currCity = nullptr;
 Connection* currConnection = nullptr;
 void getSettings();
 void saveSettings();
 void setOverlay(Overlay* ov);
 void addCity(City*);
 QList<City*> cityList;
 QMap<QString, City*> cityMap;
 QMap<QString, Overlay*>* overlayMap = new QMap<QString, Overlay*>();
 qint32 currentCityId;
 query q;
 routeView rv;
 static Configuration* instance();
 bool bDisplayWebDebug = false;
 bool bRunInBrowser = false;
 bool bShowGMFeatures =true;
 QStringList localOverlayList;
 QStringList georeferencedList;
 QString path;
 QString saveImageDir;
 QStringList cityNames();
 QString lookupCityName(Bounds b);
 QMap<QString, Bounds> cityBounds;
 bool loggingOn() {return bLoggingOn;}
 void setLoggingOn(bool b) {bLoggingOn = b;}
 bool foreignKeyCheck() {return bForeignKeyCheck;}
 void setForeignKeyCheck(bool b) {bForeignKeyCheck = b;}
 QFont font = QApplication::font();
 void changeFonts(QWidget *obj, QFont f );

private:
 static Configuration* _instance;
 explicit Configuration(QObject *parent = 0);
 void createDefaultSettings();
 bool bLoggingOn = false;
 QUuid _connectionUniqueId;
 bool bForeignKeyCheck = false;

signals:
  void newCityCreated(City*);
public slots:

};

#endif // CONFIGURATION_H
