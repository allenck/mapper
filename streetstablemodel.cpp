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
                emit dataChanged(index(row, STREETID), index(row, COMMENT));
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
        case DELETE_:
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

    connect(SQL::instance(), &SQL::segmentChanged,this,[=](SegmentInfo si, SQL::CHANGETYPE t){
        on_segmentChange(si, t);
    });

    _instance = this;
}

/*static*/ StreetsTableModel* StreetsTableModel::_instance = nullptr;

/*static*/ StreetsTableModel* StreetsTableModel::instance()
{
    if(!_instance)
        _instance = new StreetsTableModel();
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
        // case OLDERNAME:
        //     return si.olderName;
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
        // case OLDERNAME:
        //     return tr("Older Name");
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
    case STREETID:
    case LOCATION:
    case SEGMENTS:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable| Qt::ItemIsEditable;
    // case OLDERNAME:
    case NEWERNAME:
    case LENGTH:
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
bool StreetsTableModel::setData(const QModelIndex &mindex, const QVariant &value, int role)
{
    StreetInfo  sti = streetsList.at(mindex.row());
    if( role == Qt::EditRole)
    {
    switch (mindex.column()) {
        case STREET:
            sti.street = value.toString();
            break;
        case LOCATION:
            sti.location = value.toString();
            break;
        // case OLDERNAME:
        //     si.olderName = value.toString();
        //     break;
        // case NEWERNAME:
        //     si.newerName = value.toString();
        //     break;
        case STARTLATLNG:
        {
            LatLng newLatLng = LatLng::fromString(value.toString());
            if(newLatLng.isValid())
            {
                sti.startLatLng = newLatLng;
                sti.bounds = Bounds();
                sti.updateBounds();
                break;
            }
            return false;
        }
        case ENDLATLNG:
        {
            LatLng newLatLng = LatLng::fromString(value.toString());
            if(newLatLng.isValid())
            {
                sti.endLatLng = newLatLng;
                sti.bounds = Bounds();
                sti.updateBounds();
                break;
            }
            return false;
        }
        case LENGTH:
            sti.length = value.toDouble();
            break;
        case STARTDATE:
            sti.dateStart = value.toDate();
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
            sti.dateEnd = value.toDate();
            break;
        case SEGMENTS:
        {
            QStringList sl = value.toString().split(",");
            for(int i = sl.length()-1; i> 0; i--)
            {
                int segmentId = sl.at(i).toInt();
                SegmentInfo si = SQL::instance()->getSegmentInfo(segmentId);
                if(si.segmentId() != segmentId)
                    sl.removeAt(i);
                else
                {
                    si.setStreetId(sti.streetId);
                    si.setNewerName(sti.newerName);
                    SQL::instance()->updateSegment(&si);
                }
            }
            QString newList = sl.join(",");
            sti.segments = sti.setSegments(newList);
            break;
        }
        case COMMENT:
            sti.comment = value.toString();
            break;
        case SEQUENCE:
            sti.sequence = value.toInt();
        default:
            break;
        }
        streetsList.replace(mindex.row(), sti);
        //updateStreet(si);
        updateStreetDef(sti);
        emit dataChanged(index(mindex.row(), 0), index(mindex.row(), COMMENT), QList<int>());
        return true;
    }
    return false;
}

StreetInfo* StreetsTableModel::getStreetDef(int streetId)
{
    StreetInfo* si = nullptr;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, startDate, seq,rowid "
                          " from StreetDef where streetId = " + QString::number(streetId)
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
        si = new StreetInfo();
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

QList<StreetInfo*> StreetsTableModel::getStreetName(QString street, QString location, int streetId)
{
    QList<StreetInfo*> myArray;
    StreetInfo* si = nullptr;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, Seq, startDate, endDate, rowid"
                          " from StreetDef "
                          " where street = '" + street + "' and Location = '" + location + "'";
    if(streetId >0)
        commandText.append(QString(" and streetId = %1").arg(streetId));
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
        si = new StreetInfo();
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
        myArray.append(si);
    }
    return myArray;
}

bool StreetsTableModel::getStreetName(StreetInfo* sti )
{
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, Seq, startDate, endDate, rowid"
                          " from StreetDef "
                          " where street = '" + sti->street + "' and Location = '" + sti->location + "'"
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

StreetInfo* StreetsTableModel::getOlderStreet(int streetid, QString street, QString location, QDate date)
{
    StreetInfo* sti = nullptr;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, Seq, startDate, endDate, rowid"
                          " from StreetDef "
                          " where street = '" + street + "' and Location = '" + location + "'"
                          " and streetId = " +QString::number(streetid) + ""
                          " and seq > 0 ";
    if(date.isValid())
        commandText.append(" and '" + date.toString("yyyy/MM/dd") +"' between startDate and endDate");
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
        sti= new StreetInfo();
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
        if(sti->dateEnd < sti->dateStart)
            sti->dateEnd = sti->dateStart.addDays(1);
        return sti;
    }
    return nullptr;
}

QList<StreetInfo> StreetsTableModel::getStreetInfoList(QString street)
{
    QList<StreetInfo> myArray;
    StreetInfo si = StreetInfo();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, Seq, startDate, "
                          "endDate, rowid "
                          " from StreetDef ";
    if(!street.isEmpty())
        commandText.append(QString(" where street = '%1'").arg(street));
    commandText.append(" order by streetid, seq");
    if(config->currConnection->servertype() == "PostgreSQL")
        commandText.replace("rowid","0");
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return myArray;
    }
    QString curr_streetName;
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
        if(si.sequence == 0)
        {
            si.newerName.clear();
            curr_streetName = si.street;
        }
        else
            si.newerName = curr_streetName;
        myArray.append( si);
    }

    return myArray;
}

StreetInfo StreetsTableModel::getStreetInfo(int row)
{
    return streetsList.at(row);
}

bool StreetsTableModel::doesStreetDefExist(StreetInfo* sti)
{
    StreetInfo* si = nullptr;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText;
    if(sti->sequence == 0)
     commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, Seq, startDate,"
                          " endDate, rowid"
                          " from StreetDef "
                          " where street = '" + sti->street + "'"
                          " and Location = '" + sti->location + "'"
                          " and seq = " + QString::number(sti->sequence);
    else
        commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                      "Bounds,Segments,Comment, StreetId, Seq, startDate,"
                      " endDate, rowid"
                      " from StreetDef "
                      " where street = '" + sti->street + "'"
                      " and Location = '" + sti->location + "'"
                      " and seq = " + QString::number(sti->sequence) +
                      " and startDate ='" + sti->dateStart.toString("yyyy/MM/dd") +"'";
    if(config->currConnection->servertype() == "PostgreSQL")
        commandText= commandText.replace("rowid","0");

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
        sti->segments = si->setSegments(query.value(6).toString());
        sti->comment = query.value(7).toString();
        sti->streetId = query.value(8).toInt();
        sti->sequence = query.value(9).toInt();
        sti->dateStart = query.value(10).toDate();
        sti->dateEnd = query.value(11).toDate();
        sti->rowid = query.value(12).toInt();
    }
    return true;
}

int StreetsTableModel::newStreetDef(QString street, QString location, QDate date)
{
    if(street.length() > 30)
        qDebug() << "street size?";
    bool bNbr;
    if(!(street.trimmed().isEmpty()))
    {
        int num = street.toInt(&bNbr);
        if(bNbr)
            return -1;
    }
    else
        return -1;
    StreetInfo* sti = new StreetInfo();
    sti->street = street;
    sti->location = location;
    sti->sequence= 0;
    if(doesStreetDefExist(sti))
        return sti->streetId;

    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "insert into StreetDef (Street, Location, startDate) values ("
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

int StreetsTableModel::newStreetDef(StreetInfo* sti)
{
    if(sti->street.length() > 30)
        qDebug() << "street size?";
    if(sti->dateEnd.isValid())
    {
        qDebug() << "newStreetDef end date not null " << sti->dateEnd.toString("yyyy/MM/dd");
        if(doesStreetExist(sti))
            return sti->streetId;
    }
    sti->streetId = getNextStreetId();
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "insert into StreetDef (Street, Location, startDate, endDate,Seq, StreetId,"
                          "segments, startLatLng, endLatLng, Bounds, length)"
                          " values ("
                          "'" + sti->street + "',"
                          "'" + sti->location + "',"
                          "'" + sti->dateStart.toString("yyyy/MM/dd") +"',"
                          " '',"
                          "0," + QString::number(sti->streetId) + ","
                          "'" + sti->segmentsToString()  + "',"
                          "'" + sti->startLatLng.str() +"',"
                          "'" + sti->endLatLng.str() +"',"
                          "'" + sti->bounds.toString() + "',"
                          + QString::number(sti->length)
                          +")";

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
    return sti->streetId;
}

// streetid, street, location, dateStart, endDate must be populated
bool StreetsTableModel::newStreetName(StreetInfo* info)
{
    bool bNbr;
    if(!(info->street.trimmed().isEmpty()))
    {
        int num = info->street.toInt(&bNbr);
        if(bNbr)
            return -1;
    }
    else
        return -1;
    if(doesStreetDefExist(info))
    {
        qDebug() << "already exists: "<< info->toString();
        //return false;
    }

    if(info->streetId < 1 || info->street.isEmpty() || !info->dateStart.isValid())
        throw IllegalArgumentException(tr("newStreetName: invalid parameters."));
    if(info->dateEnd.isValid() && info->dateStart > info->dateEnd)
        qDebug() << "newStreetName " <<info->street << " end date " <<info->dateEnd.toString("yyyy/MM/dd")
                 << " < start date " << info->dateStart.toString("yyyy/MM/dd");

    int maxSeq = maxStreetDefSeq(info->streetId);
    if(maxSeq > info->sequence)
        info->sequence = maxSeq++;

    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "insert into StreetDef (StreetId, Street, Location,startDate, "
                          "endDate, comment, Seq, segments, startLatLng, endLatLng, Bounds, length)"

                          "values ("
                          + QString::number(info->streetId) + ","
                          "'" + info->street.trimmed() + "',"
                          "'" + info->location.trimmed() + "',"
                          "'" + info->dateStart.toString("yyyy/MM/dd") + "',"
                          "'" + info->dateEnd.toString("yyyy/MM/dd")  + "',"
                          "'" + info->comment + "',"
                          + QString::number(info->sequence) +","
                          "'" + info->segmentsToString()  + "',"
                          "'" + info->startLatLng.str() +"',"
                          "'" + info->endLatLng.str() +"',"
                          "'" + info->bounds.toString() + "',"
                          + QString::number(info->length)
                          +")";
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
    QString commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, seq, startDate, "
                          "endDate, rowid "
                          " from StreetDef "
                          " where startDate < '" + date.toString("yyyy/MM/dd") + "' "
                          " and streetId = " + QString::number(streetId);

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
    QString commandText = "insert into Streets (Street, OlderName,NewerName,StartLat, "
                          "StartLon,EndLat,EndLon,Length,"
                          "StartDate,EndDate,Segments,Comment) values ("
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
    QString commandText = " Update Streets set  OlderName ='" + si.olderName+ "',"
                          "NewerName = '" + si.newerName+ "',"
                          " StartLat = " + QString::number(si.startLatLng.lat()) +","
                          " StartLon = " + QString::number(si.startLatLng.lon()) +","
                          " EndLat = " + QString::number(si.endLatLng.lat()) +","
                          " EndLon = " + QString::number(si.endLatLng.lon()) + ","
                          " Length = " + QString::number(si.length)  + ","
                          " StartDate ='" + si.startDate.toString("yyyy/MM/dd") + "',"
                          " EndDate = '" + si.endDate.toString("yyyy/MM/dd") + "',"
                          " Segments = '" + si.segmentsToString() + "',"
                          " Comment ='" + si.comment + "'"
                          " where Street = '" + si.street + "'";

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
    bool bNbr;
    if(!(si.street.trimmed().isEmpty()))
    {
        int num = si.street.toInt(&bNbr);
        if(bNbr)
            return -1;
    }
    else
        return -1;

    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = " Update StreetDef set"
                          " Street ='" + si.street+ "',"
                          " Location = '" + si.location+ "',"
                          " startDate = '" + si.dateStart.toString("yyyy/MM/dd")+ "',"
                          " endDate = '" + si.dateEnd.toString("yyyy/MM/dd")+ "',"
                          " StartLatLng = '" + si.startLatLng.str() +"',"
                          " EndLatLng = '" + si.endLatLng.str() +"',"
                          " Bounds = '" + si.bounds.toString() + "',"
                          " Length = " + QString::number(si.length)  + ","
                          " Segments = '" + si.segmentsToString()  + "',"
                          " Comment ='" + si.comment + "',"
                          " Seq = " + QString::number(si.sequence)  + ","
                          " lastUpdate=CURRENT_TIMESTAMP"
                          " where rowid = " + QString::number(si.rowid);
    if(config->currConnection->servertype() == "PostgreSQL" )
    {
        commandText.replace("endDate = ''", "endDate = null");
    }
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
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
    bool bNbr;
    if(!(sti.street.trimmed().isEmpty()))
    {
        int num = sti.street.toInt(&bNbr);
        if(bNbr)
            return -1;
    }
    else
        return -1;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText;
    commandText = " Update StreetDef set"
                          " Street ='" + sti.street+ "',"
                          " Location = '" + sti.location+ "',"
                          " startDate = '" + sti.dateStart.toString("yyyy/MM/dd")+ "',"
                          " endDate = '" + sti.dateEnd.toString("yyyy/MM/dd")+ "',"
                          " StartLatLng = '" + sti.startLatLng.str() +"',"
                          " EndLatLng = '" + sti.endLatLng.str() +"',"
                          " Bounds = '" + sti.bounds.toString() + "',"
                          " Length = " + QString::number(sti.length)  + ","
                          " Segments = '" + sti.segmentsToString()  + "',"
                          " Comment ='" + sti.comment + "',"
                          " Seq = " + QString::number(sti.sequence)  + ","
                          " lastUpdate=CURRENT_TIMESTAMP"
                          " where rowid = " + QString::number(sti.rowid);
    if(config->currConnection->servertype() == "PostgreSQL" )
    {
        commandText.replace("endDate = ''", "endDate = null");
    }
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
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

QList<SegmentInfo> StreetsTableModel::getSegmentsForStreet(QStringList names)
{
    QList<SegmentInfo> myArray;

    QSqlDatabase db = QSqlDatabase::database();

    QString nameList;
    foreach (QString name, names) {
        nameList.append( "'" + name + "',");
    }
    if(nameList.endsWith(","))
        nameList.chop(1);
    QString commandText;
        commandText= "Select SegmentId, description, OneWay, startDate, endDate,"
                      " length, points, startLat, startLon, endLat, EndLon, type, street,"
                      " location, pointArray, tracks, direction, DoubleDate, newerName, streetId"
                      " from Segments where street in(" + nameList + ")"
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
        si._dateBegin = query.value(3).toDate();
        si._dateEnd = query.value(4).toDate();
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
        si._dateDoubled = query.value(17).toDate();
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

QList<SegmentInfo*> StreetsTableModel::getStreetsSegments(StreetInfo sti)
{
    QList<SegmentInfo*> list;
    foreach (int segmentid, sti.segments) {
        list.append(new SegmentInfo(SQL::instance()->getSegmentInfo(segmentid)));
    }
    return list;
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
        emit dataChanged(index(row,0),index(row, COMMENT), QList<int>());
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
        SQL::instance()->executeCommand(QString("delete from streetDef where rowid = %1")
                                            .arg(streetsList.at(row).rowid));
        streetsList.removeAt(row);
        endRemoveRows();
    }
}

int StreetsTableModel::findStreetId(QString street, QString location, bool bIsDef)
{
    int streetId = -1;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText;
    commandText = "select d.street, d.streetId from StreetDef d "
                  "where d.street = '" + street + "' "
                  "and d.location = '" + location + "' ";
    if(bIsDef)
        commandText = commandText.append("and Seq = 0");
    else
        commandText = commandText.append("and Seq != 0");
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

    commandText = "select street, streetId from StreetDef d "
                  "where street = '" + street + "' "
                  "and location = '" + location + "'";

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
//     QString commandText = "Select n.Street, n.Location,d.StartLatLng,d.EndLatLng, d.Length, "
//                           "d.Bounds,d.Segments,d.Comment, n.StreetId, d.street, n.startDate, "
//                           " n.endDate, n.rowid"
//                           " from StreetName n "
//                           " join StreetDef d on d.streetid = n.streetid "
//                           " order by n.streetId, n.street, n.Location";
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

    commandText = "Select Street, Location,StartLatLng,EndLatLng, Length, "
                          "Bounds,Segments,Comment, StreetId, startDate,"
                          "StartDate, endDate, Seq, rowid"
                          " from StreetDef "
                          " where streetId = " + QString::number(streetId)
                          + " order by StreetId, Seq, StartDate DESC";
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
        // if(sti->sequence > 0 && sti->sequence != i)
        // {
        //     sti->sequence = i;
        //     bUpdateNeeded = true;
        // }
        if(bUpdateNeeded)
        {
            if(sti->sequence ==0)
                updateStreetDef(*sti);
            else
                updateStreetName(*sti);
        }
    }
    return myArray;
}

// used by dialogUpdateStreets
bool StreetsTableModel::addOldStreetName(StreetInfo* sti)
{
    try {

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
    } catch (IllegalArgumentException e) {
        QMessageBox::critical(nullptr, "Error", tr("An error has occurred %1\n"
                                                   " Check streetid %2").arg(e.msg).arg(sti->streetId));
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

QStringList StreetsTableModel::getStreetnamesList(QString location)
{
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText;
    QStringList list;
    if(location.isEmpty())
        commandText = "select distinct street from StreetDef order by street";
    else
        commandText = "select distinct street from StreetDef "
                      "where location = '" + location + "' order by street";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return list;
    }
    while (query.next())
    {
        list.append(query.value(0).toString());
    }
    return list;
}

int StreetsTableModel::maxStreetDefSeq(int streetId)
{
    QSqlDatabase db = QSqlDatabase::database();
    int maxSeq = -1;
    QString commandText = "select MAX(seq) from StreetDef where StreetId = "
                          + QString::number(streetId);
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return maxSeq;
    }
    while (query.next())
    {
        maxSeq = query.value(0).toInt();
    }
    return maxSeq;
}

void StreetsTableModel::on_segmentChange(SegmentInfo si, SQL::CHANGETYPE t)
{
    int streetId = si.streetId();
    switch (t) {
    case SQL::CHANGETYPE::ADDSEG:
    case SQL::CHANGETYPE::MODIFYSEG:
    {
        if(!si.newerName().isEmpty())
        {
            if(si.streetId() < 1)
            {
                // must add newer name if not ptesent
                streetId = findStreetId(si.newerName(),si.location());
                if(streetId < 1)
                {
                    StreetInfo sti = StreetInfo();
                    sti.street = si.newerName();
                    sti.location = si.location();
                    sti.segments.append(si.segmentId());
                    sti.updateSegmentInfo(si);
                    sti.sequence = 0;
                    sti.newerName = si.newerName();
                    if(!(streetId = newStreetDef(&sti)))
                    {
                        return;
                    }
                    // now add the actual steeet
                    sti.street = si.streetName();
                    sti.sequence = 1;
                    if(!newStreetName(&sti))
                    {
                        return;
                    }
                }
                else
                {
                    processStreetUpdate(streetId, si);
                }
                SQL::instance()->executeCommand(QString("update Segments set streetId = %1 where segmentid = %3")
                                                    .arg(streetId).arg(si.segmentId()));
            }
            else
            {

                StreetInfo* sti = getStreetDef(streetId);
                if(!sti->segments.contains(si.segmentId()))
                {
                    sti->segments.append(si.segmentId());
                    sti->updateSegmentInfo(si);
                }
                if(!updateStreetDef(*sti))
                {
                    return;
                }
                processStreetUpdate(streetId, si);

            }
        }
        else {
            // The street is the current name
            if(si.streetId()>0)
            {
                //street is in the streetdef table so update the bounds
                QList<StreetInfo*> list = getStreetName(si.streetName(), si.location(), si.streetId());
                if(list.empty())
                    return;
                foreach (StreetInfo* sti, list) {
                    if(sti->segments.contains(si.segmentId())){
                        sti->bounds = Bounds();
                        for(int i=sti->segments.count()-1; i >=0; i--)
                        {
                            int segmentId = sti->segments.at(i);
                            SegmentInfo si1 = SQL::instance()->getSegmentInfo(segmentId);
                            if(si1.segmentId()<0)
                            {
                                sti->segments.removeAt(i);
                            }
                            sti->updateSegmentInfo(si1);
                        }
                        updateStreetDef(*sti);
                    }
                }
            }
        }
        break;
    }
    case SQL::CHANGETYPE::DELETESEG:
        break;
    default:
        break;
    }
}

void StreetsTableModel::processStreetUpdate(int streetId, SegmentInfo si)
{
    // add or update the actual street
    QList<StreetInfo*> list = getStreetName(si.streetName(), si.location(),streetId);
    // multiple occurrences of the name may exist because of multiple uses of the name.
    if(list.isEmpty())
    {
        StreetInfo sti = StreetInfo();
        sti.street = si.streetName();
        sti.location = si.location();
        sti.streetId = streetId;
        sti.sequence = maxStreetDefSeq(streetId)+1;
        sti.segments.append(si.segmentId());
        sti.newerName = si.newerName();
        sti.updateSegmentInfo(si);
        sti.street = si.streetName();
        if(!newStreetName(&sti))
        {
            return;
        }
    }
    if(list.count() == 1)
    {
        StreetInfo* sti = list.at(0);
        if(!sti->segments.contains(si.segmentId()))
            sti->segments.append(si.segmentId());
        sti->updateSegmentInfo(si);
        if(!updateStreetName(*sti))
        {
            return;
        }
    }
}

bool StreetsTableModel::doesStreetExist(StreetInfo* sti)
{
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText = "select streetid, seq from streetdef "
                          "where street ='" +sti->street + "' "
                          "and location = '" + sti->location +"' "
                          "and startDate ='" + sti->dateStart.toString("yyyy/MM/dd") + "' "
                          "and endDate = '" + sti->dateEnd.toString("yyyy/MM/dd") + "' ";

    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        SQLERROR(std::move(query));
        //throw Exception();
        return false;
    }
    int streetId=-1;
    int sequence =-1;
    while (query.next())
    {
        streetId = query.value(0).toInt();
        sequence = query.value(1).toInt();
    }
    if(sti->streetId <= 0)
        sti->streetId = streetId;
    return streetId > 0;
}

bool StreetsTableModel::createMissingStreetDef()
{
    QSqlDatabase db = QSqlDatabase::database();
    QList<SegmentData*> segments;
    QString where = "where streetId = -1" ;
    QList<SegmentData*> list = SQL::instance()->segmentDataListFromView(where);
    if(!list.isEmpty())
        return true;

    foreach (SegmentData* sd, segments) {
        StreetInfo info;
        info.street = sd->streetName();
        info.location = sd->location();
        info.dateStart = sd->startDate();
        info.dateEnd = sd->endDate();
        info.startLatLng = LatLng(sd->startLat(),sd->startLon());
        info.endLatLng = LatLng(sd->endLat(),sd->endLon());
        info.length = sd->length();

        if(!newStreetDef(&info))
            return false;
        if(info.streetId > 0)
        {
            sd->setStreetId(info.streetId);
            if(!SQL::instance()->updateSegment(sd))
                return false;
        }
    }
    return true;
}
