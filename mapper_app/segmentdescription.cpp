#include "segmentdescription.h"
#include "data.h"

/*static*/ QList<QPair<QString,QString>> SegmentDescription::abbreviations = QList<QPair<QString,QString>>();

SegmentDescription::SegmentDescription(QObject *parent) :
    QObject(parent)
{
    common();
}

SegmentDescription::SegmentDescription(QString description, QObject *parent) : QObject(parent)
{
    common();
    work = description;
    tokens = tokenize(work);
}

void SegmentDescription::common()
{
    Parameters parms = sql->getParameters();
    abbreviations = parms.abbreviationsList;
    _newDescription = work;
    newReverseDescription = "";
    setText(work);
}

void SegmentDescription::setText(QString descr)
{
    work = descr;
#if 0
    int len = 0;
    if (work.contains(","))
    {
        sa = work.split(',');
        // now get the from and to locations from sa[1];
        if(sa.at(1).contains(strTo) || sa.at(1).contains(strAnd) )
        {
            if (sa.at(1).contains(strTo))
            {
                i = sa.at(1).indexOf(strTo);
                len = strTo.length();
            }
            else
            {
                i = sa.at(1).indexOf(strAnd);
                len = strAnd.length();
            }
            from = sa.at(1).mid(0, i);
            to = sa.at(1).mid(i + len,-1);
            // now see if there is a (1 way) description
            if (to.contains('('))
            {
                j = to.indexOf('(');
                to = to.mid(0, j);
            }
            newReverseDescription = ((QString)sa.at(0)).trimmed() + ", " + to.trimmed() + " to " + from.trimmed() + " ";
            newDescription = sa.at(0).trimmed() + ", " + from.trimmed() + " to " + to.trimmed() + " ";
            isValid = true;
        }
    }

    else
    {
        if (work.contains(strTo) || work.contains(strAnd))
        {
            if (work.contains(strTo))
            {
                i = work.indexOf(strTo);
                len = strTo.length();
            }
            else
            {
                i = work.indexOf(strAnd);
                len = strAnd.length();
            }
            from = work.mid(0, i);
            to = work.mid(i + len, -1);
            // now see if there is a (1 way) description
            if (to.contains('('))
            {
                j = to.indexOf('(');
                to = to.mid(0, j);
            }
            if ((from.contains('&') && (to.contains('&') || to.contains(strAnd) || to.contains(strSlash)))||
                (from.contains(strAnd) && (to.contains('&') || to.contains(strAnd) || to.contains(strSlash))) ||
                (from.contains(strSlash) && (to.contains('&') || to.contains(strAnd) || to.contains(strSlash))))
            {
                if (from.contains('&'))
                    saFrom = from.split('&');
                else
                    if (from.contains(strSlash))
                    {
//                        i = from.indexOf(strSlash);
//                        len = strSlash.length();
//                        saFrom.replace(0, from.mid(0, i));
//                        saFrom.replace(1, from.mid(i + len,-1));
                        saFrom = from.split('/');
                    }
                    else
                    {
//                        i = from.indexOf(strAnd);
//                        len = strAnd.length();
//                        saFrom[0] = from.mid(0, i );
//                        saFrom[1] = from.mid(i + len, -1);
                        saFrom << from.mid(0, i ) << from.mid(i + len, -1);
                    }
                saFrom[0] = saFrom.at(0).trimmed();
                saFrom[1] = saFrom.at(1).trimmed();
                if(to.contains('&'))
                    saTo = to.split('&');
                else
                    if (to.contains(strSlash))
                    {
//                        i = to.indexOf(strSlash);
//                        len = strSlash.length();
//                        saTo[0] = to.mid(0, i);
//                        saTo[1] = to.mid(i + len,-1);
                        saTo = to.split('/');
                    }
                    else{
//                        i = to.indexOf(strAnd);
//                        len = strAnd.length();
//                        saTo[0] = to.mid(0, i );
//                        saTo[1] = to.mid(i + len,-1);
                        saTo << to.mid(0, i )<<to.mid(i + len,-1);
                    }
                saTo[0] = saTo.at(0).trimmed();
                saTo[1] = saTo.at(1).trimmed();

                if (saFrom[0] == saTo[0])   // a & b to a & c
                {
                    street = saFrom.at(0);
                    fromStreet = saFrom[1];
                    toStreet = saTo[1];
                }
                else
                    if (saFrom[0] == saTo[1])   // a & b to C & a
                    {
                        street = saFrom[0];
                        fromStreet = saFrom[1];
                        toStreet = saTo[0];
                    }
                    else  // b & a to a & c
                        if(saFrom[1] == saTo[0])
                        {
                            street = saFrom[1];
                            fromStreet = saFrom[0];
                            toStreet = saTo[1];
                        }
                        else
                            if (saFrom[1] == saTo[1])
                            {
                                street = saFrom[1];
                                fromStreet = saFrom[0];
                                toStreet = saTo[0];
                            }
                            else
                            {
                                newDescription = from + " to " + to;
                                newReverseDescription = to + " to" + from;
                                isValid = true;
                                return;
                            }

                newReverseDescription = street + ", " + toStreet.trimmed() + " to " + fromStreet.trimmed() + " ";
                newDescription = street + ", " + fromStreet.trimmed() + " to " + toStreet.trimmed() + " ";
            }
            else
            {
                newReverseDescription = street + ", " + to.trimmed() + " to " + from.trimmed() + " ";
                newDescription = street + ", " + from.trimmed() + " to " + to.trimmed() + " ";
            }
            isValid = true;
        }
    }

#endif
}
/// <summary>
/// gets the formatted description
/// </summary>
QString SegmentDescription::newDescription() {
    if(isValidFormat(work))
    {
        _newDescription = buildDescription(tokens);
    }
    return _newDescription;  }
/// <summary>
/// returns the new description with the to and from street reversed.
/// </summary>
QString SegmentDescription::reverseDescription() {
    if(isValidFormat(work))
    {
        newReverseDescription =  QString("%1, %2 - %3").arg(tokens.at(0).trimmed(),tokens.at(1).trimmed(),tokens.at(2).trimmed());
    }
    return newReverseDescription;
}
/// <summary>
/// gets the street
/// </summary>
QString SegmentDescription::street() {
    if(tokens.size() > 0)
        return tokens.at(0);
    if(!work.isEmpty())
        return work;
    return "";
}
/// <summary>
/// gets the 'to' intersecting street
/// </summary>
QString SegmentDescription::fromStreet() {
    if(tokens.size() > 1)
        return tokens.at(1);
    return "";
}
/// <summary>
/// Gets the 'from' intersecting street.
/// </summary>
QString SegmentDescription::toStreet() {
    if(tokens.size() > 2)
        return tokens.at(2);
    return "";
}
/// <summary>
/// Returns boolean value of True if the description is valid.
/// </summary>
bool SegmentDescription::isValid() {
    _isValid = isValidFormat(work);
    return _isValid;  }


bool SegmentDescription::isValidFormat(SegmentInfo si)
{
    if(si.description().isEmpty())
        qDebug() << "blank description";
    return !tokenize(si.description()).isEmpty();
}

bool SegmentDescription::isValidFormat(QString str)
{
    if(!(str.contains(",") || str.contains(" - ")|| str.contains(" to ") || str.contains(" zur ")
          || str.contains("&") /*|| !hasAbbreviations(str)*/))
        return true;
    return !tokenize(str).isEmpty();
}

QStringList SegmentDescription::tokenize(QString descr)
{
    QStringList rslt;
    QString tgt = descr.remove('.');
    QStringList sl1 = tgt.split(",");

    if(sl1.isEmpty() || sl1.count() !=2)
    {
        if(tokenizeAlternate(descr, "/").isEmpty())
            return tokenizeAlternate(descr, "&");
        rslt.append(sl1.at(0));
        return rslt;
    }
    QStringList sl2 = sl1.at(1).split(" - ");
    if( sl2.count() ==2)
    {
        rslt.append(sl1.at(0));
        rslt.append(sl2);
        return rslt;
    }
    sl2 = sl1.at(1).split(" to ");
    if(sl2.count()==2)
    {
        rslt.append(sl1.at(0));
        rslt.append(sl2);
        return rslt;
    }
    sl2 = sl1.at(1).split(" zur ");
    if(sl2.count()==2)
    {
        rslt.append(sl1.at(0));
        rslt.append(sl2);
        return rslt;
    }
    return rslt;
}

// decode alternate format: e.g. street1/street2 to street1/street3
QStringList SegmentDescription::tokenizeAlternate(QString text, QString sep)
{
    QStringList result;
    int slash1 = text.indexOf(sep);
    if(slash1 <0)
        return result;
    int slash2 = text.indexOf(sep);
    if(slash2 <0)
        return result;
    int separator;
    QString separatorTxt = " - ";
    separator = text.indexOf(separatorTxt);
    if(separator < 0)
    {
        separatorTxt = " to ";
        separator = text.indexOf(separatorTxt);
        if(separator < 0)
        {
            separatorTxt = " zur ";
            separator = text.indexOf(separatorTxt);
            if(separator < 0)
                return result;
        }
    }
    QStringList halves = text.split(separatorTxt);
    // does each half contain "/"?
    if(!(halves.at(0).contains(sep) && halves.at(1).contains(sep)))
        return result;
    QStringList tokens1 = halves.at(0).split(sep);
    QStringList tokens2 = halves.at(1).split(sep);
    if(tokens1.at(0).trimmed() == tokens2.at(0).trimmed())
    {
        result.append(tokens1.at(0));
        result.append(tokens1.at(1));
        result.append(tokens2.at(1));
        return result;
    }
    if(tokens1.at(0) == tokens2.at(1))
    {
        result.append(tokens1.at(0));
        result.append(tokens1.at(1));
        result.append(tokens2.at(0));
        return result;
    }
    if(tokens1.at(1) == tokens2.at(0))
    {
        result.append(tokens1.at(1));
        result.append(tokens1.at(0));
        result.append(tokens2.at(1));
        return result;
    }
    if(tokens1.at(1) == tokens2.at(1))
    {
        result.append(tokens1.at(1));
        result.append(tokens1.at(0));
        result.append(tokens2.at(0));
        return result;
    }
    return result;
}

QString SegmentDescription::buildDescription(QStringList tokens)
{
    if(tokens.count() != 3)
        return work;
    work = QString("%1, %2 - %3").arg(tokens.at(0).trimmed(),tokens.at(1).trimmed(),tokens.at(2).trimmed());
    return work;
}

QString SegmentDescription::replaceAbbreviations(QString descr)
{
    tokens = tokenize(descr);
    if(tokens.count() == 0)
        return descr;
    tokens.replace(0,updateToken(tokens.at(0)));
    tokens.replace(1,updateToken(tokens.at(1)));
    tokens.replace(2,updateToken(tokens.at(2)));
    return buildDescription(tokens);
}

QString SegmentDescription::updateToken(QString str){
    QStringList sl = str.split(" ");
    for(int i=0; i < sl.count(); i++)
    {
        QString result = sl.at(i);
        if(SegmentDescription::abbreviations.isEmpty())
        {
            Parameters parms = SQL::instance()->getParameters();
            SegmentDescription::abbreviations = parms.abbreviationsList;
        }

        for(const QPair<QString,QString>& pair : SegmentDescription::abbreviations)
        {
            if(result.endsWith(pair.second))
                break;
            if(result.endsWith(pair.first))
            {
                int loc = result.lastIndexOf(pair.first);
                QString remainder = result.mid(0,loc );
                result = remainder.append(pair.second);
                break;
            }
        }
        sl.replace(i,result);
    }
    return sl.join(" ");
}

bool SegmentDescription::hasAbbreviations(QString descr)
{
    tokens = tokenize(descr);
    for(const QString& token : tokens)
    {
        for(QPair<QString,QString> pair : abbreviations )
        {
            if(token.endsWith(pair.first))
            {
                return true;
            }
        }
    }
    return false;
}
