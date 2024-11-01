#ifndef EDITSEGMENTDESCR_H
#define EDITSEGMENTDESCR_H

#include <QLineEdit>
#include <QObject>
#include <functional>

class SegmentInfo;
class SQL;
class SegmentDescription;
class EditSegmentDescr : public QLineEdit
{
    Q_OBJECT
public:
    explicit EditSegmentDescr(QWidget *parent =nullptr);
    void setText(QString txt);
    void setSegmentId(int segmentId);
    bool isValidFormat();
    QString replaceAbbreviations(QString txt);
    QString streetName();
    SegmentDescription* segmentDescription(){return sd;}
    void setContextMenu(QList<QAction*> list);

private:
    QColor defaultBgColor;
    SegmentDescription* segdscr = nullptr;
    bool bTextEdited;
    SQL* sql = nullptr;
    int m_segmentId;
    SegmentDescription* sd = nullptr;
    QList<QAction*> addMenu;

public slots:
    void on_editingFinished();
    //void on_contextMenuEvent(const QPoint& pt);

signals:
    void descrUpdated(QString descr, QString street);

protected:
    std::function<void (const QPoint&)> func;
    //virtual void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // EDITSEGMENTDESCR_H
