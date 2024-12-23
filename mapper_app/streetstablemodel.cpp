#include "streetstablemodel.h"
#include "data.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "sql.h"
#include "configuration.h"
#include "streetview.h"

StreetsTableModel::StreetsTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{
    config = Configuration::instance();
    //fixDates();
    //streetsList = getStreets();
    streetsList = getStreetInfoList();
#if 0
    connect(SQL::instance(), &SQL::segmentChanged, this, [=](int segmentId){
        SegmentInfo si = SQL::instance()->getSegmentInfo(segmentId);
        StreetInfo* sti = StreetsTableModel::instance()->getStreet(si.streetName());
        QStringList names;
        QList<StreetInfo*>* namesList;
        Q_UNUSED(namesList);
        if(!sti)
        {
            sti= new StreetInfo();
            sti->street = si.getStreetName();
            if(!si.newerName().isEmpty())
                sti->newerName = sti->newerName;
            sti->startDate = si.startDate();
            sti->segments.append(si.segmentId());
            sti->updateBounds(si);
            QList<SegmentInfo> segs = getSegmentsForStreet(names);
            foreach (SegmentInfo si , segs) {
                if(!sti->segments.contains(si.segmentId()))
                    sti->segments.append(si.segmentId());
                sti->updateBounds(si);
            }
            newStreet(*sti);
            beginInsertRows(QModelIndex(), rowCount(QModelIndex()),rowCount(QModelIndex()));
            streetsList.append(*sti);
            endInsertRows();
            emit streetUpdated(rowCount(QModelIndex())-1, sti->street);
        }
        else
        {
            namesList = getStreetNames(sti->streetId, &names);

            if(!si.newerName().isEmpty() && sti->newerName != si.newerName())
                sti->newerName = si.newerName();
            if(!sti->segments.contains(si.segmentId()))
                sti->segments.append(si.segmentId());
            sti->updateBounds(si);

            if(updateStreet(*sti))
            {
                for(int row = 0; row < streetsList.count(); row++)
                {
                    StreetInfo sti1 = streetsList.at(row);
                    if(sti1.street == si.streetName())
                    {
                        streetsList.replace(row, *sti);
                        emit streetUpdated(row, sti1.street);
                        return;
                    }
                }
                streetsList.append(*sti);
                emit streetUpdated(streetsList.count() -1, si.streetName());


            }
        }
    });
#endif
    connect(this, &StreetsTableModel::streetInfoChanged, this, [=](StreetInfo si, Action act){
        int row = findRow(si.rowid);
        //QModelIndex srcIndex = index(row, 0);
        //StreetView* view = qobject_cast<StreetView*>(parent);
        //QModelIndex index =view->proxyModel->mapFromSource(srcIndex);
        //int sRow = index.row();
        switch (act) {
        case UPDATE:
            if(row >= 0)
            {
                dataChanged(index(row, STREETID), index(row, COMMENT));
              return;
            }
            break;
        case ADD:
            if(row <0)
            {
                beginInsertRows(QModelIndex(), streetsList.count(), streetsList.count());
                streetsList.append(si);
                endInsertRows();
                return;
            }
            break;
        case DELETE:
            if(row >= 0)
            {
                deleteStreetDef(row);
            }
            break;
        default:
            break;
        }
        qDebug() << QString("streetInfo %1 %3 type %2 error").arg(si.streetId).arg(actStr.at(act)).arg(si.street);
    });
    _instance = this;
}

/*static*/ StreetsTableModel* StreetsTableModel::_instance = nullptr;

/*static*/ StreetsTableModel* StreetsTableModel::instance()
{
    return _instance;
}

int StreetsTableModel::rowCount(const QModelIndex &parent) const
{
    return streetsList.count();
}
int StreetsTableModel::columnCount(const QModelIndex &parent) const
{
    return COMMENT +1;
}
QVariant StreetsTableModel::data(const QModelIndex &index, int role) const
{
    StreetInfo  si = streetsList.at(index.row());
    // if(role == Qt::CheckStateRole )
    // {
    //     if(index.column()== DEF)
    //     return si.sequence;
    // }
    if(role == Qt::BackgroundRole)
    {
        QVariant background = QVariant();
        if ( si.sequence==0)
        {
            background = QVariant( QColor("#D1FFBD"));
        }
        return background;
    }
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column()) {
        case STREETID:
            return si.streetId;
        case STREET:
            return si.street;
        case LOCATION:
            return si.location;
        case OLDERNAME:
            return si.olderName;
        case NEWERNAME:
            return si.newerName;
        case STARTLATLNG:
            return si.startLatLng.str();
        case ENDLATLNG:
            return si.endLatLng.str();
        case LENGTH:
            return si.length;
        case STARTDATE:
            return si.dateStart.toString("yyyy/MM/dd");
        case ENDDATE:
            return si.dateEnd.toString("yyyy/MM/dd");
        case SEGMENTS:
            return si.segmentsToString();
        case COMMENT:
            return si.comment;
        case SEQUENCE:
            return si.sequence;
        default:
            break;
        }
    }
    return QVariant();
}
QVariant StreetsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section) {
        case STREETID:
            return tr("Id");
        // case DEF:
        //     return tr("Def");
        case STREET:
            return tr("Street");
        case LOCATION:
            return tr("Location");
        case OLDERNAME:
            return tr("Older Name");
        case NEWERNAME:
            return tr("Newer Name");
        case STARTLATLNG:
            return(tr("Start LatLng"));
        case ENDLATLNG:
            return(tr("End LatLng"));
        case LENGTH:
            return tr("Length");
        case STARTDATE:
            return tr("Start Date");
        case ENDDATE:
            return tr("End Date");
        case SEGMENTS:
            return tr("Segments");
        case COMMENT:
            return tr("Comment");
        case SEQUENCE:
            return tr("Seq");
        default:
            break;
        }
    }
    return QVariant();

}
Qt::ItemFlags StreetsTableModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
    // case DEF:
    //     return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    case STREET:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    case OLDERNAME:
    case NEWERNAME:
    case LENGTH:
    case SEGMENTS:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    case SEQUENCE:
    case STARTDATE:
    case ENDDATE:
    case STARTLATLNG:
    case ENDLATLNG:
    case COMMENT:
    default:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }

}
bool StreetsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    StreetInfo  si = streetsList.at(index.row());
    if( role == Qt::EditRole)
    {
    switch (index.column()) {
        case STREET:
            return false;
        case OLDERNAME:
            si.olderName = value.toString();
            break;
        case NEWERNAME:
            si.newerName = value.toString();
            break;
        case STARTLATLNG:
        {
            LatLng newLatLng = LatLng::fromString(value.toString());
            if(newLatLng.isValid())
            {
                si.startLatLng = newLatLng;
                si.bounds = Bounds();
                si.updateBounds();
                break;
            }
            return false;
        }
        case ENDLATLNG:
        {
            LatLng newLatLng = LatLng::fromString(value.toString());
            if(newLatLng.isValid())
            {
                si.endLatLng = newLatLng;
                si.bounds = Bounds();
                si.updateBounds();
                break;
            }
            return false;
        }
        case LENGTH:
            si.length = value.toDouble();
            break;
        case STARTDATE:
            si.dateStart = value.toDate();
            // if(!si.olderName.isEmpty())
            // {
            //     int row = findRow(si.olderName);
            //     if(row >= 0)
            //     {
            //         StreetInfo sti = streetsList.at(row);
            //         sti.endDate = si.endDate.addDays(-1);
            //         updateStreet(sti);
            //         streetsList.replace(row, sti);
            //     }
            // }
            break;
        case ENDDATE:
            si.dateEnd = value.toDate();
            break;
        case SEGMENTS:
            si.segments = si.setSegments(value.toString());
            break;
        case COMMENT:
            si.comment = value.toString();
            break;
        case SEQUENCE:
            si.sequence = value.toInt();
        default:
            break;
        }
        streetsList.replace(index.row(), si);
        //updateStreet(si);
        updateStreetDef(si);
        return true;
    }
    return false;
}
#if 0
QList<StreetInfo> StreetsTableModel::getStreets()
{
    QList<StreetInfo> myArray;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select `Street`, `OlderName`,`NewerName`,`StartLat`, `StartLon`,"
                          "`EndLat`,`EndLon`,`Length`,"
                          "`StartDate`,`EndDate`,`Segments`,`Comment` "
                          " from Streets";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(query);
        throw Exception();
    }
    while (query.next())
    {
        StreetInfo si = StreetInfo();
        si.street = query.value(0).toString();
        si.olderName = query.value(1).toString();
        si.newerName = query.value(2).toString();
        si.startLatLng = LatLng(query.value(3).toDouble(), query.value(4).toDouble());
        si.endLatLng = LatLng(query.value(5).toDouble(), query.value(6).toDouble());
        si.length = query.value(7).toDouble();
        si.startDate = query.value(8).toDate();
        si.endDate = query.value(9).toDate();
        si.segments = si.setSegments(query.value(10).toString());
        si.comment = query.value(11).toString();
        myArray.append(si);
    }
    return myArray;
}

StreetInfo* StreetsTableModel::getStreet(QString street)
{
    StreetInfo* sti = new StreetInfo();
    sti->sequence = true;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select `Street`, `OlderName`,`NewerName`,`StartLat`, `StartLon`,`EndLat`,`EndLon`,`Length`,"
                          "`StartDate`,`EndDate`,`Segments`,`Comment` "
                          " from Streets where `street` = '" + street + "'";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(query);
        throw Exception();
    }
    while (query.next())
    {
        sti->street = query.value(0).toString();
        sti->olderName = query.value(1).toString();
        sti->newerName = query.value(2).toString();
        sti->startLatLng = LatLng(query.value(3).toDouble(), query.value(4).toDouble());
        sti->endLatLng = LatLng(query.value(5).toDouble(), query.value(6).toDouble());
        sti->length = query.value(7).toDouble();
        sti->bounds = Bounds(query.value(8).toString());
        sti->startDate = query.value(9).toDate();
        sti->endDate = query.value(10).toDate();
        sti->segments = sti->setSegments(query.value(11).toString());
        sti->comment = query.value(12).toString();
        sti->streetId = query.value(13).toInt();
        return sti;
    }
    return nullptr;
}
#endif
StreetInfo* StreetsTableModel::getStreetDef(int streetId)
{
    StreetInfo* si = new StreetInfo();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select `Street`, `Location`,`StartLatLng`,`EndLatLng`, `Length`, "
                          "`Bounds`,`Segments`,`Comment`, `StreetId`, `startDate`, `seq`,rowid "
                          " from StreetDef where `streetId` = " + QString::number(streetId)
                          + " and seq = 0";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        throw Exception();
    }
    while (query.next())
    {
        si->street = query.value(0).toString();
        si->location = query.value(1).toString();
        si->startLatLng = LatLng::fromString( query.value(2).toString());
        si->endLatLng = LatLng::fromString(query.value(3).toString());
        si->length = query.value(4).toDouble();
        si->bounds = Bounds(query.value(5).toString());
        si->segments = si->setSegments(query.value(6).toString());
        si->comment = query.value(7).toString();
        si->streetId = query.value(8).toInt();
        si->dateStart = query.value(9).toDate();
        si->sequence = query.value(10).toInt();
        si->rowid = query.value(11).toInt();
        return si;
    }
    return si;
}

StreetInfo* StreetsTableModel::getStreetName(QString street, QString location)
{
    StreetInfo* si = new StreetInfo();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select `Street`, `Location`,`StartLatLng`,`EndLatLng`, `Length`, "
                          "`Bounds`,`Segments`,`Comment`, `StreetId`, `Seq`, `startDate`, `endDate`, rowid"
                          " from StreetDef "
                          " where `street` = '" + street + "' and `Location` = '" + location + "'";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return nullptr;
    }
    while (query.next())
    {
        si->street = query.value(0).toString();
        si->location = query.value(1).toString();
        si->startLatLng = LatLng::fromString( query.value(2).toString());
        si->endLatLng = LatLng::fromString(query.value(3).toString());
        si->length = query.value(4).toDouble();
        si->bounds = Bounds(query.value(5).toString());
        si->segments = si->setSegments(query.value(6).toString());
        si->comment = query.value(7).toString();
        si->streetId = query.value(8).toInt();
        si->sequence = query.value(9).toInt();
        si->dateStart = query.value(10).toDate();
        si->dateEnd = query.value(11).toDate();
        si->rowid = query.value(12).toInt();
        return si;
    }
    return nullptr;
}

bool StreetsTableModel::getStreetName(StreetInfo* sti )
{
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select `Street`, `Location`,`StartLatLng`,`EndLatLng`, `Length`, "
                          "`Bounds`,`Segments`,`Comment`, `StreetId`, `Seq`, `startDate`, `endDate`, rowid"
                          " from StreetDef "
                          " where `street` = '" + sti->street + "' and `Location` = '" + sti->location + "'"
                          " and startDate = '" + sti->dateStart.toString("yyyy/MM/dd") +"'";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return false;
    }
    while (query.next())
    {
        sti->street = query.value(0).toString();
        sti->location = query.value(1).toString();
        sti->startLatLng = LatLng::fromString( query.value(2).toString());
        sti->endLatLng = LatLng::fromString(query.value(3).toString());
        sti->length = query.value(4).toDouble();
        sti->bounds = Bounds(query.value(5).toString());
        sti->segments = sti->setSegments(query.value(6).toString());
        sti->comment = query.value(7).toString();
        sti->streetId = query.value(8).toInt();
        sti->sequence = query.value(9).toInt();
        sti->dateStart = query.value(10).toDate();
        sti->dateEnd = query.value(11).toDate();
        sti->rowid = query.value(12).toInt();
        return true;
    }
    return false;
}

QList<StreetInfo> StreetsTableModel::getStreetInfoList()
{
    QList<StreetInfo> myArray;
    StreetInfo si = StreetInfo();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select `Street`, `Location`,`StartLatLng`,`EndLatLng`, `Length`, "
                          "`Bounds`,`Segments`,`Comment`, `StreetId`, `Seq`, `startDate`, "
                          "`endDate`, rowid "
                          " from StreetDef  order by streetid, seq";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return myArray;
    }
    while (query.next())
    {
        si.street = query.value(0).toString();
        si.location = query.value(1).toString();
        si.startLatLng = LatLng::fromString( query.value(2).toString());
        si.endLatLng = LatLng::fromString(query.value(3).toString());
        si.length = query.value(4).toDouble();
        si.bounds = Bounds(query.value(5).toString());
        si.segments = si.setSegments(query.value(6).toString());
        si.comment = query.value(7).toString();
        si.streetId = query.value(8).toInt();
        si.sequence = query.value(9).toInt();
        si.dateStart = query.value(10).toDate();
        si.dateEnd = query.value(11).toDate();
        si.rowid = query.value(12).toInt();
        myArray.append( si);
    }

    return myArray;
}

int StreetsTableModel::newStreetDef(QString street, QString location, QDate date)
{
    if(street.length() > 30)
        qDebug() << "street size?";
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "insert into StreetDef (`Street`, `Location`, `startDate`) values ("
                          "'" + street + "',"
                          "'" + location + "',"
                          "'" + date.toString("yyyy/MM/dd") +"')";

    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return false;
    }
    int rows = query.numRowsAffected();
    if (rows != 1)
        return 0;

    // Now get the streetId (identity) value so it can be returned.
    int streetId =0;
    if(config->currConnection->servertype() == "Sqlite")
        commandText = "SELECT LAST_INSERT_ROWID()";

    else
        if(config->currConnection->servertype() != "MsSql")
            commandText = "SELECT LAST_INSERT_ID()";
        else
            commandText = "SELECT IDENT_CURRENT('Segments')";
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        db.close();
        exit(EXIT_FAILURE);
    }
    while (query.next())
    {
        streetId = query.value(0).toInt();
    }
    StreetInfo si;
    si.streetId = streetId;
    si.street = street;
    si.location = location;
    si.dateStart = date;

    return streetId;
}

bool StreetsTableModel::newStreetDef(StreetInfo* sti)
{
    if(sti->street.length() > 30)
        qDebug() << "street size?";
    if(sti->dateEnd.isValid())
        qDebug() << "newStreetDef end date not null " << sti->dateEnd.toString("yyyy/MM/dd");

    sti->streetId = getNextStreetId();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "insert into StreetDef (`Street`, `Location`, `startDate`, `endDate`,`Seq`, `StreetId`) values ("
                          "'" + sti->street + "',"
                          "'" + sti->location + "',"
                          "'" + sti->dateStart.toString("yyyy/MM/dd") +"',"
                          " '',"
                          "0," + QString::number(sti->streetId) +")";

    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return false;
    }
    int rows = query.numRowsAffected();
    if (rows != 1)
        return false;

    // Now get the streetId (identity) value so it can be returned.
    if(!getStreetName(sti))
    {
        return false;
    }
    emit streetInfoChanged(*sti, ADD);
    return true;
}

// streetid, street, location, dateStart, endDate must be populated
bool StreetsTableModel::newStreetName(StreetInfo* info)
{
    if(info->streetId < 1 || info->street.isEmpty() || !info->dateStart.isValid())
        throw IllegalArgumentException(tr("newStreetName: invalid parameters."));
    if(info->dateEnd.isValid() && info->dateStart > info->dateEnd)
        qDebug() << "newStreetName " <<info->street << " end date " <<info->dateEnd.toString("yyyy/MM/dd")
                 << " < start date " << info->dateStart.toString("yyyy/MM/dd");

    if(info->sequence ==0)
        info->sequence =1;

    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "insert into StreetDef (`StreetId`, `Street`, `Location`,`startDate`, "
                          "`endDate`, `comment`, `Seq`) "
                          "values ("
                          + QString::number(info->streetId) + ","
                          "'" + info->street.trimmed() + "',"
                          "'" + info->location.trimmed() + "',"
                          "'" + info->dateStart.toString("yyyy/MM/dd") + "',"
                          "'" + info->dateEnd.toString("yyyy/MM/dd")  + "',"
                          "'" + info->comment + "',"
                          + QString::number(info->sequence) +")";

    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(query);
        //throw Exception();
        return false;
    }
    int rows = query.numRowsAffected();
    if (rows != 1)
        return false;

    if(config->currConnection->servertype() == "Sqlite")
        commandText = "SELECT LAST_INSERT_ROWID()";

    else
        if(config->currConnection->servertype() != "MsSql")
            commandText = "SELECT LAST_INSERT_ID()";
        else
            commandText = "SELECT IDENT_CURRENT('Segments')";
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        return false;
    }
    while (query.next())
    {
        info->rowid = query.value(0).toInt();
    }
    emit streetInfoChanged(*info, ADD);
    return true;
}

StreetInfo* StreetsTableModel::getEarlierStreetName(int streetId, QDate date)
{
    StreetInfo* si = new StreetInfo();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select `Street`, `Location`,`StartLatLng`,`EndLatLng`, `Length`, "
                          "`Bounds`,`Segments`,`Comment`, `StreetId`, seq, `startDate`, "
                          "`endDate`, rowid "
                          " from StreetDef "
                          " where `startDate` < '" + date.toString("yyyy/MM/dd") + "' "
                          " and `streetId` = " + QString::number(streetId);
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return nullptr;
    }
    while (query.next())
    {
        si->street = query.value(0).toString();
        si->location = query.value(1).toString();
        si->startLatLng = LatLng::fromString( query.value(2).toString());
        si->endLatLng = LatLng::fromString(query.value(3).toString());
        si->length = query.value(4).toDouble();
        si->bounds = Bounds(query.value(5).toString());
        si->segments = si->setSegments(query.value(6).toString());
        si->comment = query.value(7).toString();
        si->streetId = query.value(8).toInt();
        si->sequence = query.value(9).toInt();
        si->dateStart = query.value(10).toDate();
        si->dateEnd = query.value(11).toDate();
        si->rowid = query.value(12).toInt();
        return si;
    }

    return nullptr;
}

#if 0
bool StreetsTableModel::newStreet(StreetInfo si)
{
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "insert into Streets (`Street`, `OlderName`,`NewerName`,`StartLat`, "
                          "`StartLon`,`EndLat`,`EndLon`,`Length`,"
                          "`StartDate`,`EndDate`,`Segments`,`Comment`) values ("
                          "'" + si.street + "',"
                                        "'" + si.olderName+ "',"
                                           "'" + si.newerName+ "',"
                          + QString::number(si.startLatLng.lat()) + "," + QString::number(si.startLatLng.lon()) +","
                          + QString::number(si.endLatLng.lat()) + "," + QString::number(si.endLatLng.lon()) + ","
                          + QString::number(si.length)  + ","
                          + "'" + si.startDate.toString("yyyy/MM/dd") + "',"
                          + "'" + si.endDate.toString("yyyy/MM/dd") + "',"
                          + "'" + si.segmentsToString() + "',"
                          + "'" + si.comment+ "')";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(query);
        //throw Exception();
        return false;
    }
    int rows = query.numRowsAffected();
    if (rows == 1)
        return true;
    return false;
}

bool StreetsTableModel::updateStreet(StreetInfo si)
{
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = " Update Streets set  `OlderName` ='" + si.olderName+ "',"
                          "`NewerName` = '" + si.newerName+ "',"
                          " `StartLat` = " + QString::number(si.startLatLng.lat()) +","
                          " `StartLon` = " + QString::number(si.startLatLng.lon()) +","
                          " `EndLat` = " + QString::number(si.endLatLng.lat()) +","
                          " `EndLon` = " + QString::number(si.endLatLng.lon()) + ","
                          " `Length` = " + QString::number(si.length)  + ","
                          " `StartDate` ='" + si.startDate.toString("yyyy/MM/dd") + "',"
                          " `EndDate` = '" + si.endDate.toString("yyyy/MM/dd") + "',"
                          " `Segments` = '" + si.segmentsToString() + "',"
                          " `Comment` ='" + si.comment + "'"
                          " where `Street` = '" + si.street + "'";

    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(query);
        //throw Exception();
        return false;
    }
    int rows = query.numRowsAffected();
    if (rows == 1)
    {
        emit streetUpdated(0, si.street);
        return true;
    }
    return false;
}
#endif
bool StreetsTableModel::updateStreetName(StreetInfo si)
{

    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = " Update StreetDef set"
                          " `Street` ='" + si.street+ "',"
                          " `Location` = '" + si.location+ "',"
                          " `startDate` = '" + si.dateStart.toString("yyyy/MM/dd")+ "',"
                          " `endDate` = '" + si.dateEnd.toString("yyyy/MM/dd")+ "',"
                          " `StartLatLng` = '" + si.startLatLng.str() +"',"
                          " `EndLatLng` = '" + si.endLatLng.str() +"',"
                          " `Bounds` = '" + si.bounds.toString() + "',"
                          " `Length` = " + QString::number(si.length)  + ","
                          " `Segments` = '" + si.segmentsToString()  + "',"
                          " `Comment` ='" + si.comment + "',"
                          " `Seq` = " + QString::number(si.sequence)  +
                          " where rowid = " + QString::number(si.rowid);
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(query);
        //throw Exception();
        return false;
    }
    int rows = query.numRowsAffected();
    if (rows == 1)
    {
        //emit streetUpdated(0, si.street);
        emit streetInfoChanged(si, UPDATE);
        return true;
    }
    return false;
}
bool StreetsTableModel::updateStreetDef(StreetInfo sti)
{
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText;
    commandText = " Update StreetDef set"
                          " `Street` ='" + sti.street+ "',"
                          " `Location` = '" + sti.location+ "',"
                          " `startDate` = '" + sti.dateStart.toString("yyyy/MM/dd")+ "',"
                          " `endDate` = '" + sti.dateEnd.toString("yyyy/MM/dd")+ "',"
                          " `StartLatLng` = '" + sti.startLatLng.str() +"',"
                          " `EndLatLng` = '" + sti.endLatLng.str() +"',"
                          " `Bounds` = '" + sti.bounds.toString() + "',"
                          " `Length` = " + QString::number(sti.length)  + ","
                          " `Segments` = '" + sti.segmentsToString()  + "',"
                          " `Comment` ='" + sti.comment + "',"
                          " `Seq` = " + QString::number(sti.sequence)  +
                          " where rowid = " + QString::number(sti.rowid);

    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(query);
        //throw Exception();
        return false;
    }
    int rows = query.numRowsAffected();
    if (rows == 1)
    {
        //emit streetUpdated(0, sti.street);
        emit streetInfoChanged(sti, UPDATE);
        return true;
    }
    return false;
}

QList<SegmentInfo> StreetsTableModel::getSegmentsForStreet(QStringList nameList)
{
    QList<SegmentInfo> myArray;

    QSqlDatabase db = QSqlDatabase::database();


    QString commandText;
        commandText= "Select SegmentId, description, OneWay, startDate, endDate,"
                      " length, points, startLat, startLon, endLat, EndLon, type, street,"
                      " location, pointArray, tracks, direction, DoubleDate, newerName, streetId"
                      " from Segments where street in('" + nameList.join(",") + "')"
                      + "order by description";

    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        qDebug() << "Is default database correct?";
        //            db.close();
        //            exit(EXIT_FAILURE);
        return myArray;
    }
    while (query.next())
    {
        SegmentInfo si;
        si._segmentId = query.value(0).toInt();
        si._description = query.value(1).toString();
        //si._oneWay = query.value(2).toString();
        si._startDate = query.value(3).toDate();
        si._endDate = query.value(4).toDate();
        si._length = query.value(5).toDouble();
        si._points = query.value(6).toInt();
        si._startLat = query.value(7).toDouble();
        si._startLon = query.value(8).toDouble();
        si._endLat = query.value(9).toDouble();
        si._endLon = query.value(10).toDouble();
        si._bearing = Bearing(si._startLat, si._startLon, si._endLat, si._endLon);
        si._direction = si._bearing.strDirection();
        si._routeType = (RouteType)query.value(11).toInt();
        si._streetName = query.value(12).toString();
        si._location = query.value(13).toString();
        QString pointArray = query.value(14).toString();
        si._tracks = query.value(15).toInt();
        si.direction() = query.value(16).toString();
        si._doubleDate = query.value(17).toDate();
        si._newerStreetName = query.value(18).toString();
        si._streetId = query.value(19).toInt();

        si.setPoints(pointArray);  // initialize array of points (i.e pointList)
        if(si.pointList().count() > 1)
        {
            si._bearingStart = Bearing(si._startLat, si._startLon, si.pointList().at(1).lat(), si.pointList().at(1).lon());
            si._bearingEnd = Bearing(si.pointList().at(si._points-2).lat(), si.pointList().at(si._points-2).lon(), si._endLat, si._endLon);
        }

        si._bounds = Bounds(pointArray);
        if(si.description().isEmpty())
        {
            si._description = si._bearing.strDirection();
            si._bNeedsUpdate = true;
        }
        if(si._streetName.isEmpty() || si._streetName.indexOf("(")> 0)
        {
            QStringList tokens = si._description.split(",");
            if(tokens.count() > 1)
            {
                QString street = tokens.at(0).trimmed();
                street= street.mid(0, street.indexOf("("));
                si._streetName = street;
                si._bNeedsUpdate = true;
            }
            else
            {
                tokens = si._description.split(" ");
                if(tokens.count() > 1)
                {
                    QString street = tokens.at(0).trimmed();
                    street= street.mid(0, street.indexOf("("));
                    si._streetName = street;
                    si._bNeedsUpdate = true;
                }
            }
        }
        myArray.append(si);
    }

    return myArray;
}

void StreetsTableModel::streetChanged(StreetInfo info)
{
    // update SegmentData with segment changes.
    int row = -1;
    for(int i=0; i < streetsList.count(); i++)
    {
        StreetInfo sti= streetsList.at(i);
        if(info.street == sti.street)
        {
            row = i;
            break;
        }
    }
    if(row >=0)
    {
        streetsList.replace(row, info);
        dataChanged(index(row,0),index(row, COMMENT), QList<int>());
    }
    else
    {
        beginInsertRows(QModelIndex(),row,row);
        streetsList.append(info);
        endInsertRows();
    }
}

void StreetsTableModel::setList(QList<StreetInfo> streets)
{
    beginResetModel();
    streetsList = streets;
    endResetModel();
}
#if 0
void StreetsTableModel::deleteStreet(QString street)
{
    int row = findRow(street);
    if(row >=0)
    {
        beginRemoveRows(QModelIndex(), row,row);
        streetsList.removeAt(row);
        endRemoveRows();
    }
}
#endif
int StreetsTableModel::findRow(int rowid)
{
    int row = -1;
    for(int i=0; i < streetsList.count(); i++)
    {
        StreetInfo sti= streetsList.at(i);
        if(sti.rowid == rowid )
        {
            row = i;
            break;
        }
    }
    return row;
}

void StreetsTableModel::deleteStreetDef(int row)
{
    //int row = findRow(sti.street);
    if(row >=0)
    {
        beginRemoveRows(QModelIndex(), row,row);
        streetsList.removeAt(row);
        endRemoveRows();
    }
}

int StreetsTableModel::findStreetId(QString street, QString location)
{
    int streetId = -1;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText;
    commandText = "select d.`street`, d.`streetId` from `StreetDef` d "
                  "where d.`street` = '" + street + "' "
                  "and d.`location` = '" + location + "'";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        qDebug() << "Is default database correct?";
        //            db.close();
        //            exit(EXIT_FAILURE);
        return streetId;
    }
    while (query.next())
    {
        streetId = query.value(1).toInt();
    }
    if(streetId >= 0)
        return streetId;

    commandText = "select `street`, `streetId` from `StreetDef` d "
                  "where `street` = '" + street + "' "
                  "and `location` = '" + location + "'";

    if(!bQuery)
    {
        SQLERROR(std::move(query));
        qDebug() << "Is default database correct?";
        //            db.close();
        //            exit(EXIT_FAILURE);
        return streetId;
    }
    while (query.next())
    {
        streetId = query.value(1).toInt();
    }
    return streetId;
}

bool StreetsTableModel::fixDates()
{
    QStringList names;
    QList<StreetInfo*>* list;
    QDate maxDate = QDate::fromString("2050/01/01", "yyyy/MM/dd");
    bool bMustUpdate;
    int maxStreetid = getNextStreetId();

    for(int i=1; i < maxStreetid; i++)
    {
        qApp->processEvents();
        list = getStreetNames(i, &names);
        for(int j = 0; j < list->count(); j++)
        {
            bMustUpdate=false;
            StreetInfo* sti = list->at(j);

            if(sti->sequence == 0 &&  sti->dateEnd == maxDate)
            {
                sti->dateEnd = QDate();
                bMustUpdate=true;
            }
            if(j+1 < list->count())
            {
                StreetInfo* nxtSti = list->at(j+1);

                if(!(sti->dateStart >  nxtSti->dateStart))
                {
                    nxtSti->dateStart = sti->dateStart.addDays(-1);
                    bMustUpdate=true;
                    nxtSti->dateEnd = maxDate.addDays(-1);
                    if(!updateStreetName(*nxtSti))
                    {
                        qDebug() << "updateStreetName failed";
                        return false;
                    }
                }
            }
            if((bMustUpdate))
            {
                if(sti->sequence == 0)
                {
                    if(!updateStreetDef(*sti))
                    {
                        qDebug() << "updateStreetName failed";
                        return false;
                    }
                }
                else
                if(!updateStreetName(*sti))
                {
                    qDebug() << "updateStreetName failed";
                    return false;
                }
            }
        }
    }
    return true;
}
// One time function to remove duplicates
// bool StreetsTableModel::fixStreetName()
// {
//     int rowId = -1;
//     StreetInfo* prevSi = new StreetInfo();
//     QMap<int, int> toKeep;
//     QList<int> toDelete;
//     StreetInfo* si = new StreetInfo();
//     QSqlDatabase db = QSqlDatabase::database();
//     QString commandText = "Select n.`Street`, n.`Location`,d.`StartLatLng`,d.`EndLatLng`, d.`Length`, "
//                           "d.`Bounds`,d.`Segments`,d.`Comment`, n.`StreetId`, d.street, n.`startDate`, "
//                           " n.`endDate`, n.rowid"
//                           " from StreetName n "
//                           " join StreetDef d on d.streetid = n.streetid "
//                           " order by n.`streetId`, n.`street`, n.`Location`";
//     QSqlQuery query = QSqlQuery(db);
//     bool bQuery = query.exec(commandText);
//     if(!bQuery)
//     {
//         SQLERROR(query);
//         //throw Exception();
//         return false;
//     }
//     while (query.next())
//     {
//         si->street = query.value(0).toString();
//         si->location = query.value(1).toString();
//         si->startLatLng = LatLng::fromString( query.value(2).toString());
//         si->endLatLng = LatLng::fromString(query.value(3).toString());
//         si->length = query.value(4).toDouble();
//         si->bounds = Bounds(query.value(5).toString());
//         si->segments = si->setSegments(query.value(6).toString());
//         si->comment = query.value(7).toString();
//         si->streetId = query.value(8).toInt();
//         si->newerName = query.value(9).toString();
//         si->startDate = query.value(10).toDate();
//         si->endDate = query.value(11).toDate();
//         int thisRowId = query.value(12).toInt();
//         if(!toKeep.contains(si->streetId))
//             toKeep.insert(si->streetId,thisRowId);
//         else
//         {
//             toDelete.append(thisRowId);
//         }
//     }

//     //now delete the excess entries
//     foreach(int rowid, toDelete)
//     {
//         commandText = "delete from streetname where rowid = " +QString::number(rowid);
//         if(!SQL::instance()->executeCommand(commandText,db))
//             return false;
//     }

//     return true;
// }

QList<StreetInfo*>* StreetsTableModel::getStreetNames(int streetId, QStringList *nameList)
{
    QList<StreetInfo*>* myArray = new QList<StreetInfo*>;

    StreetInfo* si = new StreetInfo();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText;

    commandText = "Select `Street`, `Location`,`StartLatLng`,`EndLatLng`, `Length`, "
                          "`Bounds`,`Segments`,`Comment`, `StreetId`, IIF(seq =0,'2050/01/01',`StartDate`),"
                          "`StartDate`, `endDate`, `Seq`, rowid"
                          " from StreetDef "
                          " where `streetId` = " + QString::number(streetId)
                          + " order by `StreetId`, IIF(seq =0,'2050/01/01',`StartDate`) DESC";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return nullptr;
    }
    while (query.next())
    {
        si =new StreetInfo();
        si->street = query.value(0).toString();
        si->location = query.value(1).toString();
        si->startLatLng = LatLng::fromString( query.value(2).toString());
        si->endLatLng = LatLng::fromString(query.value(3).toString());
        si->length = query.value(4).toDouble();
        si->bounds = Bounds(query.value(5).toString());
        si->segments = si->setSegments(query.value(6).toString());
        si->comment = query.value(7).toString();
        si->streetId = query.value(8).toInt();
        si->dateSort = query.value(9).toDate();
        si->dateStart = query.value(10).toDate();
        si->dateEnd = query.value(11).toDate();
        si->sequence = query.value(12).toInt();
        si->rowid = query.value(13).toInt();
        if(nameList)
            nameList->append(si->street);

        myArray->append(si);
    }
    // scan thru the list and adjust the dates and sequence numbers.
    for(int i=0; i < myArray->count(); i++)
    {
        StreetInfo* sti = myArray->at(i);
        QDate prevStartDate;
        if(i > 0)
             prevStartDate = myArray->at(i-1)->dateStart;
        bool bUpdateNeeded = false;
        if(sti->dateEnd != prevStartDate.addDays(-1))
        {
            sti->dateEnd = prevStartDate.addDays(-1);
            bUpdateNeeded = true;
        }
        if(sti->sequence != i && sti->sequence > 0)
        {
            sti->sequence = i;
            bUpdateNeeded = true;
        }
        if(bUpdateNeeded)
            updateStreetDef(*sti);
    }
    return myArray;
}

bool StreetsTableModel::addOldStreetName(StreetInfo* sti)
{
    // streetid, street, StartDate must be populated. If added, StreetInfo will be updated with enddate.
    QStringList names;
    QList<StreetInfo*>* streetNames = getStreetNames(sti->streetId, &names);
    int streetId = sti->streetId;
    //StreetInfo* currentSti = getStreetDef(sti->streetId);
    //QList<SegmentInfo> segList = getSegmentsForStreet(names);
    if(!streetNames)
        return false;
    // foreach (SegmentInfo si, segList) {
    //     currentSti->updateBounds(si);
    //     if(!sti->segments.contains(si.segmentId()))
    //         sti->segments.append(si.segmentId());
    // }
    // if(!updateStreetDef(*currentSti))
    //     return false;
    if(names.isEmpty())
    {
        return newStreetName(sti); // just add it
    }
    // See if this is a new street
    if(!names.contains(sti->street))
    {
        StreetInfo* stiExisting = nullptr;
        for(int i=0; i< streetNames->count(); i++)
        {
          stiExisting = streetNames->at(i);
          if(sti->dateStart < stiExisting->dateStart)
              continue;
          sti->dateEnd = stiExisting->dateStart.addDays(-1);
          sti->sequence = i;
          if(newStreetName(sti))
          {
              return false;
          }
          streetNames->insert(i, sti);
          sti = nullptr;
        }
        if(sti)
        {
            sti->dateEnd = stiExisting->dateStart.addDays(-1);
            sti->sequence =streetNames->count();
            if(newStreetName(sti))
            {
                return false;
            }
            streetNames->append(sti);
        }
        streetNames = getStreetNames(streetId, &names); // this will take care of resequencing etc.
    }
    else
    {
        // special case to determine whether the current street's start date needs to be altered
        StreetInfo* currentSti = streetNames->at(0);
        StreetInfo* newSti = streetNames->at(1);
        if(sti->street == currentSti->street && !currentSti->encompasses(*sti))
        {
            //currentSti can't contain this date. We must lower start date and newSti's end date
            currentSti->dateStart = sti->dateStart;
            newSti->dateEnd = sti->dateStart.addDays(-1);

            updateStreetDef(*currentSti);
            updateStreetName(*newSti);

            // get he list again because some dates may have benn corrected
            streetNames = getStreetNames(sti->streetId, &names);
            return true;
        }

        for(int i=0; i< streetNames->count(); i++)
        {
            StreetInfo* stiExisting = streetNames->at(i);
            if(!(sti->street == stiExisting->street && sti->location == stiExisting->location))
                continue;
            if( stiExisting->encompasses(*sti))
            {
                // already exists
                return false;
            }
            if(sti->dateStart < stiExisting->dateStart)
                continue;
            if(stiExisting->inDateRange(sti->dateStart))
            {
                sti->dateEnd =stiExisting->dateEnd;
                stiExisting->dateEnd = sti->dateStart.addDays(-1);

                if(!updateStreetName(*stiExisting))
                {
                    return false;
                }
                //sti falls within date range of existing street record
                if(!newStreetName(sti))
                {
                    return false;
                }
                return true;
            }
        }
#if 0
        // need to create a new one
        if(i >0)
        {
            // update the prior name's end date
            StreetInfo* prior = streetNames->at(i-1);
            prior->endDate = sti->startDate.addDays(-1);
            if(!updateStreetName(*prior))
                return false;
        }
        if((i+1) < streetNames->count() )
        {
           StreetInfo* next = streetNames->at(i*1);
           sti->endDate = next->startDate.addDays(-1);
           sti->newerName = next->street;
        }
        return newStreetName(sti);
#endif
    }
    return true;
}

int StreetsTableModel::getNextStreetId()
{
    int streetId = -1;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "select max(streetid) from streetdef";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return -1;
    }
    while (query.next())
    {
        streetId = query.value(0).toInt();
    }
    return streetId +1;
}
