#ifndef GEODBSQL_H
#define GEODBSQL_H

#include <QObject>
#include "sql.h"

class geodbObject
{
public:
    qint32 id_PLZ;
    QString PLZ;
    qint32 idStadt;
    QString Stadt;
    qint32 idBezirk;
    QString Bezirk;
    qint32 idOrsteil;
    QString Ortstiel;
    qint32 idStrasse;
    QString Strasse;
    QString Strasse_von;
    QString Strasse_bis;
    qint32 Strasse_type;
    qint32 idLatLong;
    QString LatLong_von;
    QString LatLon_bis;
    double latitude;
    double longitude;
};

class GeodbSql : public QObject
{
    Q_OBJECT
public:
    explicit GeodbSql(QObject *parent = 0);
    QList<geodbObject> getStreetList(QString text);

signals:

public slots:

private:
    QSqlDatabase db;

};

#endif // GEODBSQL_H
