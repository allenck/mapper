#ifndef SEGMENTDESCRIPTION_H
#define SEGMENTDESCRIPTION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <iostream>
#include <QDebug>

class SegmentDescription : public QObject
{
    Q_OBJECT
public:
    explicit SegmentDescription(QObject *parent = 0);
    SegmentDescription(QString Description);
    QString NewDescription();
    QString ReverseDescription();
    QString Street();
    QString FromStreet();
    QString ToStreet();
    bool IsValid();

signals:

public slots:
private:
    QString work;
    QStringList sa;
    QStringList saFrom;
    QStringList saTo;
    QString street;
    QString fromStreet;
    QString toStreet;

    QString from;
    QString to;
    QString newDescription;
    QString newReverseDescription;
    int i, j;
    bool isValid;
    bool isDirty;

};

#endif // SEGMENTDESCRIPTION_H
