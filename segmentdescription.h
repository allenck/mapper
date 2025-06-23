#ifndef SEGMENTDESCRIPTION_H
#define SEGMENTDESCRIPTION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDebug>
#include "sql.h"
#include "configuration.h"

class SegmentDescription : public QObject
{
    Q_OBJECT
public:
    explicit SegmentDescription(QObject *parent = 0);
    SegmentDescription(QString Description, QObject *parent = 0);
    QString newDescription();
    QString reverseDescription();
    QString street();
    QString fromStreet();
    QString toStreet();
    bool isValid();
    static QList<QPair<QString,QString>> abbreviations;
    bool isValidFormat(SegmentInfo si);
    bool isValidFormat(QString str);
    SQL* sql = SQL::instance();
    Configuration * config = Configuration::instance();
    QStringList tokenize(QString descr);
    QString buildDescription(QStringList tokens);
    QString replaceAbbreviations(QString descr);
    static QString updateToken(QString str);
    static SegmentDescription* instance();
    QStringList tokenizeAlternate(QString text, QString sep);
    void setText(QString descr);
    bool hasAbbreviations(QString descr);

signals:

public slots:
private:
    QString work;
    QString _newDescription;
    QString newReverseDescription;
    int i, j;
    bool _isValid;
    bool isDirty;
    //static SegmentDescription* _instance;
    void common();
    // QString strTo = " to ";
    // QString strAnd = " and ";
    // QString strAmp = " & ";
    // QString strSlash = "/";
    QStringList tokens;
};

#endif // SEGMENTDESCRIPTION_H
