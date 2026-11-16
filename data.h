#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QtCore/qmath.h>
#include <QStringList>
#include <QPersistentModelIndex>
#include <QRectF>
#include "latlng.h"
#include "qjsonarray.h"
#include "routedata.h"
#include <QUrl>
#include "segmentdata.h"
#include "segmentinfo.h"


class RouteSeq
{
 public:
  RouteSeq() {}
  ~RouteSeq() {}
  RouteSeq(const RouteSeq&);
  QList<QPair<int, QString>> seqList() {return _seqList;}
  QString routeName() {return _rd.routeName();}
  void setRouteName(QString name) {_rd.setRouteName(name);}
  int route() {return _rd.route();}
  QDate startDate() {return _rd.startDate();}
  QDate endDate() {return _rd.endDate();}
  QString whichEnd() {return _whichEnd;}
  int firstSegment() {return _firstSegment;}
  QString listString();

 private:
  QList<QPair<int, QString>> _seqList;
  int _firstSegment;
  QString _whichEnd;
  RouteData _rd =RouteData();
  QDateTime _lastUpdate;
  friend class SQL;
};


class segmentGroup
{
    public:
        //explicit segmentGroup(QObject *parent = 0);
        Q_DECL_DEPRECATED segmentGroup();
        qint32	SegmentId;
        QString Description;
        QString OneWay;
        double distance;
        //qint32	lineSegments;
        QString startDate;
        QString endDate;
        qint32	tractionType;
        RouteType routeType;
        double startLat, startLon, endLat, endLon;
        QList<SegmentData> data ;      // array of segmentdata objects
        QList<LatLng> points;
        ~segmentGroup();
};

class RouteInfo
{
	public:
        explicit RouteInfo(QObject *parent = 0);
        RouteInfo(qint32 route, QString name, QDate startDate, QDate endDate, int companyKey,
                  QString alphaRoute, int routeId = -1);
        ~RouteInfo() {}
        RouteInfo(RouteData rd);
        RouteInfo(RouteInfo& o);
        RouteInfo(const SegmentData &o);

        //QList<segmentGroup> segments;  // array of segmentGroup objects
        void setRouteId(int routeId){_routeId = routeId;}
        int routeId()const {return _routeId;}
        void setRouteName(QString name) {routeName = name;}

 private:
        qint32	route=-1;
        QString routeName;
        QString _companyName;
        qint32	tractionType;
        QList<SegmentData*> segmentDataList;
        double length;
        RouteData rd;
        QDate startDate;
        QDate endDate;
        int companyKey=0;
        QString alphaRoute;
        int baseRoute=0;
        QString routePrefix;
        QString companyMnemonic;
        int _routeId = -1;
        friend class SQL;
};

class TerminalInfo
{
    public:
        TerminalInfo()
        {
            route = -1;
            startSegment = -1;
            endSegment = -1;
        }

        qint32 route;
        QString name;
        QDate startDate;
        QDate endDate;
        qint32 startSegment;
        QString startWhichEnd;
        qint32 endSegment;
        QString endWhichEnd;
        LatLng startLatLng;
        LatLng endLatLng;
};

class TractionTypeInfo
{
    public:
        TractionTypeInfo(){}
        qint32 tractionType;
        QString description;
        QString displayColor;
        RouteType routeType;
        QString icon;
        QString ToString()
        {
            return QString("%1").arg(tractionType) + " " + description;
        }
};

class RouteIntersects
{
    public:
    explicit RouteIntersects(QObject *parent = 0){Q_UNUSED(parent)}
        SegmentData sd;
        QList<SegmentData> startIntersectingSegments;
        QList<SegmentData> endIntersectingSegments;
};

// Stations are some type of transit stop whether it be a Rapid Transit or railroad station or a streetcar or bus stop.
// Stations are associated with a point on a segment of a specific route type. This enables us to
// differentiate say between a streetcar stop at the same location as a subway stop.
class StationInfo
{
    public:
        //explicit stationInfo(QObject *parent = 0);
        StationInfo(){stationKey = -1;}
        ~StationInfo() {}
        StationInfo(const StationInfo&);
        QStringList routes;
        QString stationName;
        QString stationSuffix; // if multiple stops exist at say the intersection of two lines, each stop can be identified with a suffix code.
        double latitude;       // latitude of the station
        double longitude;      // longitude of the station.
        qint32 stationKey;
        qint32 infoKey;        // key to the comments table containing descritptive text for the station.
        QDate startDate;
        QDate endDate;
        RouteType routeType;
        QString markerType;
        int segmentId = -1;
        QStringList segments; // list of segments
};

class CompanyData
{
    public:
    CompanyData(QObject *parent =0)
    {
     Q_UNUSED(parent)
     routePrefix = "";
     companyKey = -1;
    }
    CompanyData(const CompanyData& o)
    {
        companyKey = o.companyKey;
        name = o.name;
        mnemonic = o.mnemonic;
        routePrefix = o.routePrefix;
        startDate = o.startDate;
        endDate = o.endDate;
        firstRoute =o.firstRoute;
        lastRoute = o.lastRoute;
        info = o.info;
        url = o.url;
        bSelected =o.bSelected;
    }

    QString toString()
    {
     return QString("%1").arg(companyKey) + " " + name;
    }

    int companyKey;
    QString name;
    QDate startDate, endDate;
    int firstRoute, lastRoute;
    QString routePrefix;
    QString mnemonic;
    QString info;
    QUrl url;
    QDateTime lastUpdated;
    bool bSelected;
};

class CommentInfo
{
public:
    CommentInfo(){commentKey = -1;}
    CommentInfo(int commentKey) {this->commentKey = commentKey;}
    ~CommentInfo() {}
    CommentInfo(const CommentInfo& other);
    qint32 commentKey = -1;
    QString tags;
    QString comments;
    QStringList usedByRoutes;
    int routeCount;
    QList<int> routesUsed = QList<int>();
    QList<int> toRoutesTable(QString routeList);
    QStringList toAroutesList(QJsonArray aRoutes);
    static QString routesTableToString(QList<int> table);
    QString jRoutesTableToString(QList<QString> table); // create jRoutesListString from table
    QString aRoutesToString();  // create jRoutesListString from stringlist aRoutesList
    void populateARoutes(QString jRouteList);
    QJsonArray aRoutes = QJsonArray();
    QStringList aRoutesList;   // list of aRoutes
    QString jRoutesListString; // string representation of QJsonArray, e.g ["81","82"]
    QDate date;
    QString alphaRoute;
    qint32 route = -1;
    LatLng pos;
    QString routeName;
    qint32 companyKey;
    qint32 routeId;
    QString companyName;
};

class RowChanged
{
public:
    qint32 row;
    QPersistentModelIndex index;
    bool bChanged = false;
    bool bDeleted = false;
    qint32 segmentId;
    SegmentData osd;
    SegmentData sd;
    RowChanged()
    {
     //si = SegmentInfo();
     segmentId = -1;
     row = -1;
     bChanged = false;
     bDeleted = false;
     index = QPersistentModelIndex();
    }
    RowChanged(int row,SegmentData sd )
    {
     this->row = row;
     this->sd = SegmentData(sd);
     bDeleted = true;
    }
    RowChanged(int row,bool bChanged, SegmentData osd, SegmentData sd)
    {
     this->row = row;
     this->bChanged = bChanged;
     this->sd = SegmentData(sd);
     this->osd =SegmentData(osd);
    }
};

class RouteComments
{
public:
    qint32 route=-1;
    QDate date;
    qint32 commentKey=-1;
    qint32 routeId = -1;
    CommentInfo ci;
    LatLng pos;
    qint32 companyKey=-1;
    QString routeName;
    QString routeAlpha;
    QString companyName;
    QT_DEPRECATED RouteComments(){}
    ~RouteComments() {}
    QT_DEPRECATED RouteComments(const RouteComments&other);
};

class RouteChangedEventArgs
{
public:
    RouteChangedEventArgs()
    {

    }

    RouteChangedEventArgs(RouteData rd, int segmentId, QString type)
    {
     typeOfChange = type;
     this->rd = rd;
     routeNbr = rd.route();
     this->routeName = rd.routeName();
     this->routeSegment = segmentId;
     tractionType = rd.tractionType();
     companyKey =rd.companyKey();
     dateEnd = rd.endDate();
    }

    RouteChangedEventArgs(QString type, qint32 s, qint32 tt)
    {
     typeOfChange = type;
     routeSegment = s;
     tractionType =tt;

    }

    RouteChangedEventArgs( qint32 r, QString n, qint32 s, qint32 tt, qint32 ck, QDate de, QString type)
    {
     routeNbr = r;
     routeName = n;
     routeSegment = s;
     tractionType = tt;
     companyKey =ck;
     dateEnd = de;
     typeOfChange = type;
    }
    qint32 routeNbr =-1;
    QString routeName;
    qint32 routeSegment = -1;
    qint32 tractionType;
    qint32 companyKey;
    QDate dateEnd;
    QString typeOfChange;
    RouteData rd;
};

struct FKInfo
{
  QString name;
  int id = 0;
  int seq = 0;
  QString table;
  QString from;
  QString to;
  QString on_update;
  QString on_delete;
  QString match;
};

Q_DECLARE_METATYPE(RouteType)

class StreetInfo
{
public:
    QString street;
    QString olderName;
    QString newerName;
    LatLng startLatLng;
    LatLng endLatLng;
    double length = 0;
    QDate dateStart;
    QDate dateEnd;
    QDate dateSort;
    QList<int> segments;
    QString comment;
    Bounds bounds;
    int streetId = -1;
    QString location;
    int sequence = 0;
    int rowid =-1;
    StreetInfo() {}
    ~StreetInfo() {}
    StreetInfo(const StreetInfo&);
    QList<int> setSegments(const QString);
    QString segmentsToString();
    void updateSegmentInfo(SegmentInfo si);
    void updateBounds();
    bool inDateRange(QDate dt);
    bool encompasses(StreetInfo sti);
    QString toString();
};
Q_DECLARE_METATYPE(StreetInfo)
#endif // DATA_H
