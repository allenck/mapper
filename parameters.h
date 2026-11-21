#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <QString>
#include <QDate>

class Parameters
{
public:
    Parameters();
    QString title;
    QString city;
    double lat, lon;
    QDate minDate, maxDate;
    bool bAlphaRoutes= false;
    QString abbreviationsString;
    QString versionString;
    void buildList(QString string);
    QString createString();
    QList<QPair<QString, QString>> abbreviationsList(){return _abbreviationsList;}
    void setList(QList<QPair<QString, QString>> al){_abbreviationsList = al;}
    void setDateFormat(QString dateFormat){_dtFormat = dateFormat;}
    QString dateFormat() {return _dtFormat;
    }
private:
    QList<QPair<QString, QString>> _abbreviationsList;
    QString _dtFormat = "yyyy/MM/dd";
    friend class SQL;
};

#endif // PARAMETERS_H
