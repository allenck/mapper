#ifndef STREETSTABLEMODEL_H
#define STREETSTABLEMODEL_H

#include <QAbstractTableModel>
#include "data.h"
#include "sql.h"
#include <QTextEdit>

class Configuration;
class StreetsTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    // bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    // bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    enum COLUMNS
    {
        STREETID,
        // DEF,
        STREET,
        LOCATION,
        STARTDATE,
        ENDDATE,
        SEQUENCE,
        // OLDERNAME,
        NEWERNAME,
        STARTLATLNG,
        ENDLATLNG,
        LENGTH,
        SEGMENTS,
        COMMENT
    };
    enum Action
    {
        ADD,
        DELETE_,
        UPDATE,
    };
    QStringList actStr = {"Add", "Delete", "Update"};
    //QList<StreetInfo> getStreets();
    //StreetInfo* getStreet(QString street);
    //bool newStreet(StreetInfo si);
    //bool updateStreet(StreetInfo si);
    static StreetsTableModel* instance();
    QList<SegmentInfo> getSegmentsForStreet(QStringList names);
    void streetChanged(StreetInfo info);
    void setList(QList<StreetInfo> streets);
    //void deleteStreet(QString street);
    void deleteStreetDef(int row);
    int findRow(int rowid);
    QT_DEPRECATED int newStreetDef(QString street, QString location = "", QDate date = QDate::fromString("2050/01/01"));
    int newStreetDef(StreetInfo* sti);
    bool newStreetName(StreetInfo *info);
    int findStreetId(QString street, QString location="", bool bIsDef=true);
    bool updateStreetDef(StreetInfo);
    StreetInfo* getStreetDef(int streetId);
    QList<StreetInfo *> getStreetName(QString street, QString location = "", int streetId =-1);
    bool getStreetName(StreetInfo* sti );
    bool updateStreetName(StreetInfo si);
    StreetInfo* getEarlierStreetName(int streetid, QDate date);
    //bool fixStreetName();
    QList<StreetInfo *> *getStreetNames(int streetId, QStringList *nameList=nullptr);
    bool addOldStreetName(StreetInfo* sti);
    QList<StreetInfo> getStreetInfoList(QString street = "");
    StreetInfo getStreetInfo(int row);
    int getNextStreetId();
    bool fixDates();
    StreetInfo *getOlderStreet(int streetid, QString Street, QString location, QDate date);
    QStringList getStreetnamesList(QString location);
    bool doesStreetDefExist(StreetInfo* sti);
    int maxStreetDefSeq(int streetId);
    void on_segmentChange(SegmentInfo si, SQL::CHANGETYPE t);
    bool doesStreetExist(StreetInfo* sti);
    QList<SegmentInfo*> getStreetsSegments(StreetInfo sti);

signals:
    void streetUpdated(int row, QString street);
    void streetInfoChanged(StreetInfo sti, Action act);

private:
    explicit StreetsTableModel(QObject *parent = nullptr);

    QList<StreetInfo> streetsList;
    static StreetsTableModel* _instance;
    Configuration* config = nullptr;
    void processStreetUpdate(int streetId, SegmentInfo si);

};

#endif // STREETSTABLEMODEL_H
