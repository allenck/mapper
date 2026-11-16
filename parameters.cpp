#include "parameters.h"

Parameters::Parameters() {}
void Parameters::buildList(QString string)
{
    QStringList sl = string.split(",");
    _abbreviationsList.clear();
    foreach(QString s, sl)
    {
        QStringList sl = s.split(":");
        if(sl.count() == 2)
            _abbreviationsList.append(QPair<QString,QString>(sl.at(0),sl.at(1)));
    }
}
QString Parameters::createString()
{
    QString str;
    for (QPair<QString,QString> pair: _abbreviationsList) {
        str.append(pair.first);
        str.append(":");
        str.append(pair.second);
        str.append(",");
    }
    return str;
}
