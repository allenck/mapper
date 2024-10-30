#ifndef EDITSEGMENTDESCR_H
#define EDITSEGMENTDESCR_H

#include <QLineEdit>
#include <QObject>

class SegmentInfo;
class SQL;
class SegmentDescription;
class EditSegmentDescr : public QLineEdit
{
public:
    EditSegmentDescr(QWidget *parent =0);
    void setText(QString txt);
    void setSegmentId(int segmentId);
    bool isValidFormat();
    QString replaceAbbreviations(QString txt);
    QString streetName();
    SegmentDescription* segmentDescription(){return sd;}

private:
    QColor defaultBgColor;
    SegmentDescription* segdscr = nullptr;
    bool bTextEdited;
    SQL* sql = nullptr;
    int m_segmentId;
    SegmentDescription* sd =nullptr;

public slots:
    void on_editingFinished();
};

#endif // EDITSEGMENTDESCR_H
