#include "segmentdescription.h"

SegmentDescription::SegmentDescription(QObject *parent) :
    QObject(parent)
{
}
SegmentDescription::SegmentDescription(QString Description)
{
    work = Description;
    newDescription = work;
    newReverseDescription = "";
    QString strTo = " to ";
    QString strAnd = " and ";
    QString strAmp = " & ";
    QString strSlash = "/";
    QStringList saFrom =  QStringList();
    QStringList saTo =  QStringList();

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
}
/// <summary>
/// gets the formatted description
/// </summary>
QString SegmentDescription::NewDescription() {  return newDescription;  }
/// <summary>
/// returns the new description with the to and from street reversed.
/// </summary>
QString SegmentDescription::ReverseDescription() {  return newReverseDescription;  }
/// <summary>
/// gets the street
/// </summary>
QString SegmentDescription::Street() { return street; }
/// <summary>
/// gets the 'to' intersecting street
/// </summary>
QString SegmentDescription::FromStreet() {return fromStreet; }
/// <summary>
/// Gets the 'from' intersecting street.
/// </summary>
QString SegmentDescription::ToStreet() { return toStreet; }
/// <summary>
/// Returns boolean value of True if the description is valid.
/// </summary>
bool SegmentDescription::IsValid() { return isValid;  }


//private string stripIt(string str)
//{
//    while (str.StartsWith(" "))
//    {
//        str = str.Substring(1, str.Length - 1);
//    }
//    while (str.EndsWith(" "))
//    {
//        str = str.Substring(0, str.Length - 1);
//    }
//    return str;
//}
