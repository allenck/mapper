#ifndef DIALOGEDITSTREETS_H
#define DIALOGEDITSTREETS_H

#include <QDialog>
#include "data.h"
#include "sql.h"
#include "configuration.h"
#include "segmentdescription.h"

namespace Ui {
class DialogEditStreets;
}

class DialogEditStreets : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditStreets(QWidget *parent = nullptr);
    ~DialogEditStreets();

private:
    Ui::DialogEditStreets *ui;
    QList<QPair<QString,QString>> abbreviations;
    // bool isValidFormat(SegmentInfo si);
    SQL* sql = SQL::instance();
    Configuration * config = Configuration::instance();
    // QStringList tokenize(QString descr);
    // QString buildDescription(QStringList tokens);
    // QString updateToken(QString str);

private slots:
    void replaceAbbreviations(bool refresh=true);
    void on_cbSegmentsCustomContextMenuRequested(const QPoint &pos);
};

#endif // DIALOGEDITSTREETS_H
