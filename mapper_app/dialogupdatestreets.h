#ifndef DIALOGUPDATESTREETS_H
#define DIALOGUPDATESTREETS_H

#include "qtextedit.h"
#include <QDialog>

namespace Ui {
class DialogUpdateStreets;
}

class Configuration;
class SegmentViewTableModel;
class QSortFilterProxyModel;
class StreetsTableModel;
class DialogUpdateStreets : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUpdateStreets(QWidget *parent = nullptr);
    ~DialogUpdateStreets();
    void closeEvent(QCloseEvent *) override;

private:
    Ui::DialogUpdateStreets *ui;
    QStringList streets;
    Configuration* config = nullptr;
    QMap<int, QString> footnoteMap;
    QMap<QString, QString> abbrevMap; // full name, abbreviated name
    QMap<QString,int> streetMap;
    int footnotedStreets;
    int maxFootnote;
    StreetsTableModel* streetsTableModel = nullptr;
    bool nextClicked = false;
    bool buttonClicked = false;
    bool prevClicked = false;
    bool abortClicked = false;
    bool bValidating = false;
    QString test1 = "Falkenberger Str.1 – Wartenberger Str. – Hauptstr. – Berliner Str.2 – Hohenschönhauser Str. – Oderbruchstr. – Landsberger Allee – Landsberger Str. – Alexanderplatz (zurück: Alexanderstr. – Kaiserstr. – Kleine Frankfurter Str. – Landsberger Str.) – Königstr.3 – Spandauer Str. – Mühlendamm – Köllnischer Fischmarkt – Gertraudenstr. – Spittelmarkt – Leipziger Str. – Jerusalemer Str. – Lindenstr. – Belle-Alliance-Platz4 – Hallesches Tor – Belle-Alliance-Brücke5 – Blücherplatz – Belle-Alliance-Str.6 – Yorckstr. – Bülowstr. – Nollendorfplatz – Kleiststr. – Wittenbergplatz – Tauentzienstr. – Auguste-Viktoria-Platz7 – Hardenbergstr. – Am Knie8 – Berliner Str.9 – Luisenplatz – Schloßbrücke – Tauroggener Str. – Osnabrücker Str. – Tegeler Weg – Nonnendamm – Siemensdamm – Nonnendammallee – Paulsternstr. – Gartenfelder Str. – Daumstr. – Berliner Ch.10 – Berliner Str.10 – Havelstr. – Potsdamer Str.11 (zurück: Markt – Breite Str. – Berliner Str.10) – Seegefelder Str.";
    QString test2 = "1 Gehrenseestr.  2 Konrad-Wolf-Str.  3 Rathausstr.   4 Mehringplatz   5 Hallesche-Tor-Brücke   6 Mehringdamm"
                    "7 Breitscheidplatz    8 Ernst-Reuter-Platz   9 Otto-Suhr-Allee    10 Am Juliusturm  11 Carl-Schurz-Str.";
    void segmentViewContextMenu(const QPoint pt);
    SegmentViewTableModel* segmentViewSourceModel = nullptr;
    QSortFilterProxyModel* segmentViewProxyModel = nullptr;
    void resizeEvent(QResizeEvent *e) override;
    void textEditChanged();
    void footnotesChanged();
    void pbUpdateClicked();
    int curIndex=-1;
    int indexOfDigit(QString txt, int begin);
    QMapIterator<QString, int>* iter = nullptr;
    void lookupStreet(QTextEdit *edit, QString st);

};

#endif // DIALOGUPDATESTREETS_H
