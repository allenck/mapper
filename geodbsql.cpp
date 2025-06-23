#include "geodbsql.h"

GeodbSql::GeodbSql(QObject *parent) :
    QObject(parent)
{
    db = QSqlDatabase::addDatabase("QMYSQL", "geodb_berlin");
    db.setDatabaseName("geodb_berlin");
    db.setHostName("ubuntu-2");
    db.setUserName("allen");
    db.setPassword("knobacres");
    if(! db.open())
    {
        qDebug()<< db.lastError();
        //return false;
    }
}
QList<geodbObject> GeodbSql::getStreetList(QString text)
{
    QList<geodbObject> geodbObjectList;
    QString CommandText = "SELECT DISTINCT plz_code.loc_id AS id_PLZ, plz_code.text_val AS PLZ, stadt.loc_id AS id_Stadt, stadt.text_val AS Stadt, bezirk.loc_id AS id_Bezirk, bezirk.text_val AS Bezirk, ortsteil.loc_id AS id_Ortsteil, ortsteil.text_val AS Ortsteil, street.loc_id AS id_Strasse, street.text_val AS Strasse, street.valid_since AS Strasse_Von, street.valid_until AS Strasse_Bis, street.text_type AS Strasse_Typ, coord.loc_id AS id_LatLong, coord.valid_since AS LatLong_Von, coord.valid_until AS LatLong_Bis, coord.lat AS Latitude, coord.lon AS Longitude FROM geodb_textdata_berlin street, geodb_textdata_berlin plz_code_such, geodb_textdata_berlin plz_code, geodb_textdata_berlin ortsteil_such, geodb_textdata_berlin ortsteil, geodb_textdata_berlin bezirk_such, geodb_textdata_berlin bezirk, geodb_textdata_berlin stadt_such, geodb_textdata stadt, geodb_coordinates_berlin coord WHERE  ((street.text_val RLIKE :searchText) AND (street.text_type >= 101100000) AND (street.text_type <= 101400000)) AND ((plz_code_such.loc_id = street.loc_id) AND  (plz_code_such.text_type = 400100000)) AND ((plz_code.loc_id = plz_code_such.text_val) AND  (plz_code.text_type = 500300000)) AND ((ortsteil_such.loc_id = street.loc_id) AND  (ortsteil_such.text_type = 400100000)) AND ((ortsteil.loc_id = ortsteil_such.text_val) AND  (ortsteil.text_type = 101000000)) AND ((bezirk_such.loc_id = ortsteil.loc_id) AND  (bezirk_such.text_type = 400100000)) AND ((bezirk.loc_id = bezirk_such.text_val) AND  (bezirk.text_type = 100900000)) AND ((stadt_such.loc_id = bezirk.loc_id) AND  (stadt_such.text_type = 400100000)) AND ((stadt.loc_id = stadt_such.text_val) AND  (stadt.text_type = 500100000)) AND ((coord.loc_id = street.loc_id))  AND  (((street.valid_since >= '0001-01-01') AND (street.valid_until <= '3000-01-01'))  AND  ((plz_code_such.valid_since < street.valid_until) AND (plz_code_such.valid_until >= street.valid_until))  AND  ((plz_code.valid_since < street.valid_until) AND (plz_code.valid_until >= street.valid_until))  AND  ((ortsteil_such.valid_since < street.valid_until) AND (ortsteil_such.valid_until >= street.valid_until))  AND  ((ortsteil.valid_since < street.valid_until) AND (ortsteil.valid_until >= street.valid_until))  AND  ((bezirk_such.valid_since < street.valid_until) AND (bezirk_such.valid_until >= street.valid_until))  AND  ((bezirk.valid_since < street.valid_until) AND (bezirk.valid_until >= street.valid_until))  AND  ((stadt_such.valid_since < street.valid_until) AND (stadt_such.valid_until >= street.valid_until))  AND  ((stadt.valid_since < street.valid_until) AND (stadt.valid_until >= street.valid_until))  AND  ((coord.valid_since < street.valid_until) AND (coord.valid_until >= street.valid_until)))  UNION SELECT DISTINCT 0 AS id_PLZ, 'na' AS PLZ, stadt.loc_id AS id_Stadt, stadt.text_val AS Stadt, bezirk.loc_id AS id_Bezirk, bezirk.text_val AS Bezirk, ortsteil.loc_id AS id_Ortsteil, ortsteil.text_val AS Ortsteil, transport.loc_id AS id_Strasse, transport.text_val AS Strasse, transport.valid_since AS Strasse_Von, transport.valid_until AS Strasse_Bis, transport.text_type AS Strasse_Typ, coord.loc_id AS id_LatLong, coord.valid_since AS LatLong_Von, coord.valid_until AS LatLong_Bis, coord.lat AS Latitude, coord.lon AS Longitude FROM geodb_textdata_berlin_transport transport, geodb_textdata_berlin_transport ortsteil_such, geodb_textdata_berlin ortsteil, geodb_textdata_berlin bezirk_such, geodb_textdata_berlin bezirk, geodb_textdata_berlin stadt_such, geodb_textdata stadt, geodb_coordinates_berlin coord WHERE  ((transport.text_val RLIKE :searchText) AND (transport.text_type > 101100000) AND (transport.text_type <= 101400000)) AND ((ortsteil_such.loc_id = transport.loc_id) AND  (ortsteil_such.text_type = 400100000)) AND ((ortsteil.loc_id = ortsteil_such.text_val) AND  (ortsteil.text_type = 101000000)) AND ((bezirk_such.loc_id = ortsteil.loc_id) AND  (bezirk_such.text_type = 400100000)) AND ((bezirk.loc_id = bezirk_such.text_val) AND  (bezirk.text_type = 100900000)) AND ((stadt_such.loc_id = bezirk.loc_id) AND  (stadt_such.text_type = 400100000)) AND ((stadt.loc_id = stadt_such.text_val) AND  (stadt.text_type = 500100000)) AND ((coord.loc_id = transport.loc_id))  AND  (((transport.valid_since >= '0001-01-01') AND (transport.valid_until <= '3000-01-01'))  AND  ((ortsteil_such.valid_since < transport.valid_until) AND (ortsteil_such.valid_until >= transport.valid_until))  AND  ((ortsteil.valid_since < transport.valid_until) AND (ortsteil.valid_until >= transport.valid_until))  AND  ((bezirk_such.valid_since < transport.valid_until) AND (bezirk_such.valid_until >= transport.valid_until))  AND  ((bezirk.valid_since < transport.valid_until) AND (bezirk.valid_until >= transport.valid_until))  AND  ((stadt_such.valid_since < transport.valid_until) AND (stadt_such.valid_until >= transport.valid_until))  AND  ((stadt.valid_since < transport.valid_until) AND (stadt.valid_until >= transport.valid_until))  AND  ((coord.valid_since < transport.valid_until) AND (coord.valid_until >= transport.valid_until)))  UNION SELECT DISTINCT 0 AS id_PLZ, 'na' AS PLZ, land.loc_id AS id_Stadt, land.text_val AS Stadt, kreis.loc_id AS id_Bezirk, kreis.text_val AS Bezirk, name.loc_id AS id_Ortsteil, name.text_val AS Ortsteil, transport2.loc_id AS id_Strasse, transport2.text_val AS Strasse, transport2.valid_since AS Strasse_Von, transport2.valid_until AS Strasse_Bis, transport2.text_type AS Strasse_Typ, coord.loc_id AS id_LatLong, coord.valid_since AS LatLong_Von, coord.valid_until AS LatLong_Bis, coord.lat AS Latitude, coord.lon AS Longitude FROM geodb_textdata_berlin_transport transport2, geodb_textdata_berlin_transport name_such, geodb_textdata name, geodb_textdata kreis, geodb_textdata kreis_such, geodb_textdata land_such, geodb_textdata land, geodb_coordinates_berlin coord WHERE  ((transport2.text_val RLIKE :searchText) AND (transport2.text_type > 101100000) AND (transport2.text_type <= 101400000)) AND ((name_such.loc_id = transport2.loc_id) AND  (name_such.text_type = 400100000)) AND ((name.loc_id = name_such.text_val) AND  (name.text_type = 500100000)) AND ((kreis_such.loc_id = name.loc_id) AND  (kreis_such.text_type = 400100000)) AND ((kreis.loc_id = kreis_such.text_val) AND  (kreis.text_type = 500100000)) AND ((land_such.loc_id = kreis.loc_id) AND  (land_such.text_type = 400100000)) AND ((land.loc_id = land_such.text_val) AND  (land.text_type = 500100000)) AND ((coord.loc_id = transport2.loc_id))  ORDER BY Stadt, Strasse, PLZ, Bezirk, Ortsteil";
    QSqlQuery query = QSqlQuery(db);
    if(!query.prepare(CommandText))
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    }
    //query.bindValue(":searchText", "'"+text+"'");
    CommandText.replace(":searchText", "'"+text+"'");
    //qDebug()<<CommandText;
    bool bQuery = query.exec(CommandText);
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        db.close();
        exit(EXIT_FAILURE);
    }
    while(query.next())
    {
        geodbObject go;
        go.id_PLZ = query.value(0).toInt();
        go.PLZ = query.value(1).toString();
        go.idStadt = query.value(2).toInt();
        go.Stadt = query.value(3).toString();
        go.idBezirk = query.value(4).toInt();
        go.Bezirk = query.value(5).toString();
        go.idOrsteil=query.value(6).toInt();
        go.Ortstiel = query.value(7).toString();
        go.idStrasse = query.value(8).toInt();
        go.Strasse = query.value(9).toString();
        go.Strasse_von = query.value(10).toString();
        go.Strasse_bis = query.value(11).toString();
        go.Strasse_type = query.value(12).toInt();
        go.idLatLong = query.value(13).toInt();
        go.LatLong_von = query.value(14).toString();
        go.LatLon_bis = query.value(15).toString();
        go.latitude = query.value(16).toDouble();
        go.longitude = query.value(17).toDouble();
        geodbObjectList.append(go);
    }
    return geodbObjectList;
}
