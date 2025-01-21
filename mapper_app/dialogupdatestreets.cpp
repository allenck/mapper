#include "dialogupdatestreets.h"
#include "qmenu.h"
#include "segmentviewtablemodel.h"
#include "ui_dialogupdatestreets.h"
#include "streetstablemodel.h"
#include "segmentdescription.h"
#include <QTableView>
#include "sql.h"
#include "webviewbridge.h"

DialogUpdateStreets::DialogUpdateStreets(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogUpdateStreets)
{
    ui->setupUi(this);
    streetsTableModel = StreetsTableModel::instance();
    config = Configuration::instance();
    ui->groupBox->setVisible(false);
    if(!config->dus.geometry.isEmpty())
        restoreGeometry(config->dus.geometry);

    connect(ui->textEdit, &QTextEdit::textChanged, this, [=]{
        try{
            textEditChanged();
        }
    catch(Exception e)
        {
            ui->textEdit->setText("");
            ui->lblHelp->setText("Invalid input. Please re-enter");
        }
    });

    connect(ui->textEdit_2, &QTextEdit::textChanged, this, [=]{
        try {
            footnotesChanged();
        }
    catch(IllegalArgumentException e)
        {
            ui->lblHelp->setText(tr(" error: %1").arg(e.msg));
        }
    });
    if(!config->dus.text1.isEmpty())
    {
        ui->textEdit->setHtml(config->dus.text1);
        ui->textEdit_2->setHtml(config->dus.text2);
        ui->dateEdit->setDate(config->dus.date);
    }

    connect(ui->pbUpdate, &QPushButton::clicked,this, [=]{
        pbUpdateClicked();
    });

    connect(ui->pbCancel,&QPushButton::clicked,this, [=]{
        buttonClicked = true;
        curIndex = -1;
        close();
    });

    connect(ui->pbNext, &QPushButton::clicked,this,[=]{
        nextClicked = true;
        buttonClicked = true;
    });

    connect(ui->pbPrev, &QPushButton::clicked,this,[=]{
        prevClicked = true;
        buttonClicked = true;
    });

    connect(ui->pbAbort, &QPushButton::clicked,this,[=]{
        abortClicked = true;
        buttonClicked = true;
    });

    connect(ui->pbUpdateSegments, &QPushButton::clicked,this, [=]{
        if(segmentViewSourceModel)
        {
            setCursor(Qt::WaitCursor);
            ui->lblHelp->clear();
            ui->lblInfo->clear();
            ui->lblInfo->setStyleSheet("color: green");

            QList<SegmentInfo>* list = segmentViewSourceModel->getList();
            int updated = 0;
            StreetInfo* sti = nullptr;
            int streetId = -1;
            for (int row =0; row < list->count(); row++) {
                SegmentInfo si = list->at(row);
                if(si.streetId() < 0)
                {
                    streetId = streetsTableModel->findStreetId(si.streetName(), si.location());
                    if(streetId && si.streetId() == -1)
                    {
                        si.setStreetId(streetId);
                        sti = streetsTableModel->getStreetDef(si.streetId());
                        if(sti)
                        {
                            si.setNewerName(sti->street);
                            sti->updateSegmentInfo(si);
                            sti->startLatLng = sti->bounds.swPt();
                            sti->endLatLng = sti->bounds.nePt();
                            sti->length = SQL::instance()->Distance(sti->startLatLng, sti->endLatLng);
                            if(!sti->segments.contains(si.segmentId()))
                                sti->segments.append(si.segmentId());
                            SQL::instance()->updateSegment(&si);
                            streetsTableModel->updateStreetDef(*sti);
                            list->replace(row, si);
                            updated++;
                        }
                        SQL::instance()->updateSegment(&si);
                    }
                }
            }
            if(updated ==list->count())
                ui->lblHelp->setStyleSheet("color: green");
            else
                ui->lblHelp->setStyleSheet("color: red");
            ui->lblHelp->setText(tr("updated %1 of %2").arg(updated).arg( list->count()));
            setCursor(Qt::ArrowCursor);
        }
        //nextClicked = true;
        qApp->processEvents();
        ui->pbNext->click();
    });

    connect(ui->segmentView->horizontalHeader(), &QHeaderView::sectionResized, this,
        [=](int logicalIndex, int oldSize, int newSize){
        config->dus.state = ui->segmentView->horizontalHeader()->saveState();
        if(!config->dus.colWidths.isEmpty())
            config->dus.colWidths.replace(logicalIndex,newSize);
    });
    //ui->segmentView->horizontalHeader()->restoreState(config->dus.state);

    if(!config->dus.colWidths.isEmpty())
    {
        for(int i=0; i < config->dus.colWidths.count(); i++)
        {
            ui->segmentView->setColumnWidth(i, config->dus.colWidths.at(i));
        }
    }

    // for testing purposes set some test data;
    // ui->textEdit->setText(test1);
    // ui->txtFootnotes->setText(test2);

    setWindowTitle(tr("Update streets"));
}

DialogUpdateStreets::~DialogUpdateStreets()
{
    delete ui;
}

void DialogUpdateStreets::closeEvent(QCloseEvent *)
{
    buttonClicked = true;
}

void DialogUpdateStreets::segmentViewContextMenu(const QPoint pt)
{
    QMenu menu;
    int curRow = ui->segmentView->rowAt(pt.y());
    int curCol = ui->segmentView-> columnAt(pt.x());
    if(curRow < 0 || curCol < 0)
        return;
    QItemSelectionModel * selectionModel =ui->segmentView-> selectionModel();
    QModelIndexList indexes = selectionModel->selectedIndexes();
    if(indexes.isEmpty())
        return;
    QModelIndex ix_segmentId = indexes.at(SegmentViewTableModel::SEGMENTID);
    QModelIndex six_segmentId = segmentViewProxyModel->mapToSource(ix_segmentId);
    int curSrcRow = six_segmentId.row();
    //int segmentId = ix_segmentId.data().toInt();
    int segmentId = ix_segmentId.model()->index(ix_segmentId.row(),SegmentViewTableModel::SEGMENTID ).data().toInt();

    QAction* act = new QAction(tr("remove segment %1").arg(segmentId), this);
    connect(act, &QAction::triggered,this, [=]{
        if(segmentViewSourceModel)
        {
            segmentViewSourceModel->removeRows(curSrcRow,1);
        }
    });
    menu.addAction(act);
    act = new QAction(tr("Show street bounds"), this);
    connect(act, &QAction::triggered, this, [=]{
        SegmentInfo si = SQL::instance()->getSegmentInfo(segmentId);
        if(si.streetId()> 0)
        {
            StreetInfo* sti = streetsTableModel->getStreetDef(si.streetId());
            WebViewBridge::instance()->processScript("clearRectangle");
            QVariantList objArray;
            objArray << sti->bounds.nePt().lat() << sti->bounds.swPt().lat() << sti->bounds.nePt().lon()
                     << sti->bounds.swPt().lon() << "FF00FF";
            WebViewBridge::instance()->processScript("showRectangle", objArray);
        }
    });
    menu.addAction(act);
    menu.exec(QCursor::pos());
}
void DialogUpdateStreets::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    config->dus.geometry = saveGeometry();
}

void DialogUpdateStreets::textEditChanged()
{
    if(bValidating)
        return;
    QString text = ui->textEdit->toPlainText();
    bValidating = true;
    text.remove('\n');
    // scan for possible format errors
    curIndex = 1;
    int length = text.length();
    QTextCursor cursor;
    cursor.setPosition(1);
    ui->textEdit->setTextCursor(cursor);
    QString selection;
    bool bError = false;
    while(curIndex > 0)
    {
        cursor = ui->textEdit->textCursor();
        curIndex = cursor.position();
        qApp->processEvents();
        if(ui->textEdit->find("–"))
        {
            cursor = ui->textEdit->textCursor();
            curIndex = cursor.position();
            cursor.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,2);
            cursor.setPosition(curIndex+1, QTextCursor::KeepAnchor);
            ui->textEdit->setTextCursor(cursor);
            selection = cursor.selectedText();
        }
        else
        {
            curIndex = -1;
            break;
        }
        if(selection.at(0) == ' ' && selection.at(2) == ' ' )
        {
            continue; // valid!
        }
        if(selection.at(0).isLower() && selection.at(2).isUpper() )
        {
           continue; // valid!
        }
        if(selection.at(0) == ' ' && selection.at(2).isUpper() )
        {
            // cursor.clearSelection();
            // cursor.movePosition(QTextCursor::Right);
            // cursor.insertText(" ");

            QMessageBox::critical(nullptr, tr("Error"), tr("An error occurred at position %1 near '%2'")
                                                        .arg(cursor.position()).arg(selection));

            bError = true;
            curIndex = -1;
        }
    }
    if(bError)
    {
        bValidating = false;
        return;
    }
    streets = text.split(" – ");
    streetMap.clear();
    QChar c;
    foreach (QString s, streets) {
        int ix = s.length()-1;
        while(ix > 0 )
        {
            c = s.at(ix);
            if(!c.isDigit())
                break;
            ix --;
        }
        if(/*c == "(" ||*/ c == ')')
            ix++;
        if(ix > 0)
        {
            int nxtSp = s.indexOf(" ");
            QString ss = s.mid(ix+1, nxtSp - ix);
            int ssn = ss.toInt();
            QString st = s.mid(0,ix+1);
            st.replace(".","");
            st.replace(")","");
            st.replace("0","");
            st.replace("1","");
            st.replace("2","");
            st.replace("3","");
            st.replace("4","");
            st.replace("5","");
            st.replace("6","");
            st.replace("7","");
            st.replace("8","");
            st.replace("9","");
            if(st.contains("(zurück:"))
            {
                QStringList sl = st.split("(zurück:");
                QString ss1;
                int len = sl.at(0).length();
                QChar c = sl.at(0).at(len-1);
                if(c.isDigit())
                {
                    ss1.append(c);
                    c = sl.at(0).at(len-2);
                    if(c.isDigit())
                        ss1.prepend(c);
                }
                streetMap.insert(SegmentDescription::updateToken(sl.at(0).trimmed()), ss1.toInt());
                abbrevMap.insert(SegmentDescription::updateToken(sl.at(0).trimmed()),sl.at(0));
                lookupStreet(ui->textEdit, abbrevMap.value(sl.at(0).trimmed()));
                streetMap.insert(SegmentDescription::updateToken(sl.at(1).trimmed()), ss.toInt());
                abbrevMap.insert(SegmentDescription::updateToken(sl.at(1).trimmed()),sl.at(1));
                lookupStreet(ui->textEdit, abbrevMap.value(sl.at(1).trimmed()));
            }
            else
            {
                streetMap.insert(SegmentDescription::updateToken(st.trimmed()), ss.toInt());
                abbrevMap.insert(SegmentDescription::updateToken(st.trimmed()),st.trimmed());
                lookupStreet(ui->textEdit, abbrevMap.value(st.trimmed()));
            }
            if(ssn > 0)
                qDebug() << st << " " << ssn;
        }
    }
    footnotedStreets =0;
    foreach (int i, streetMap.values()) {
        if(i>footnotedStreets)
            footnotedStreets = i;
    }
    bValidating = false;
}

void DialogUpdateStreets::lookupStreet(QTextEdit* edit,QString st)
{
    if(st.isEmpty())
        return;
    QString fullName = SegmentDescription::updateToken(st);
    QTextCursor currCursor = edit->textCursor();
    if(streetsTableModel->findStreetId(fullName) < 0)
    {
        QTextCursor cursor;
        cursor.setPosition(0);
        edit->setTextCursor(cursor);
        if(edit->find(st))
            edit->setTextColor(Qt::darkMagenta);
    }
    else
    {
        QTextCursor cursor;
        cursor.setPosition(0);
        edit->setTextCursor(cursor);
        if(edit->find(st))
            edit->setTextColor(Qt::black);
    }
    edit->setTextCursor(currCursor);
    qApp->processEvents();
}

void DialogUpdateStreets::footnotesChanged()
{
    if(bValidating)
        return;
    bValidating =true;
    ui->lblHelp->clear();
    QString text = ui->textEdit_2->toPlainText();
    QString digits;
    QString street;
    int ix =0;
    int iy = 0;
    QChar c;
    ui->lblHelp->setText("");
    qApp->processEvents();
    footnoteMap.clear();
    text.remove('\n');
    if(text.isEmpty() || !text.at(0).isDigit())
    {
        ui->lblHelp->setStyleSheet("color: red");
        ui->lblHelp->setText("invalid footnotes");
        return;
    }
    while(ix >= 0)
    {
        c= text.at(ix);
        ix++;
        if(c.isDigit())
        {
            digits.append(c);
            if(digits.length()>2)
                throw IllegalArgumentException(tr("bad input %1").arg(text));
            continue;
        }
        iy = indexOfDigit(text, ix);
        if(iy < 0)
        {
            street = text.mid(ix);
            ix = -1;
        }
        else
        {
            street = text.mid(ix, iy-ix);
            ix = iy;
        }
        QString st = street.remove(".");
        street = SegmentDescription::updateToken(street.trimmed());
        footnoteMap.insert(digits.toInt(),street.trimmed());
        lookupStreet(ui->textEdit_2,st.trimmed());
        digits.clear();
    }

    bValidating=false;

}

void DialogUpdateStreets::pbUpdateClicked()
{
    maxFootnote =0;
    ui->lblHelp->setText("");
    config->dus.text1 = ui->textEdit->toHtml();
    config->dus.text2 = ui->textEdit_2->toHtml();
    config->dus.date = ui->dateEdit->date();
    abortClicked = false;
    buttonClicked = false;
    nextClicked = false;
    prevClicked = false;

    // ui->groupBox->setVisible(true);
    // ui->segmentView->setVisible(true);
    foreach(int fn, streetMap.values())
    {
        if(fn > maxFootnote)
            maxFootnote = fn;
    }
    if(maxFootnote != footnotedStreets)
    {
        ui->lblHelp->setStyleSheet("color: red");
        ui->lblHelp->setText(tr("% fn streets vs %2 footnotes!").arg(maxFootnote,footnotedStreets ));
    }
    ui->pbUpdate->setEnabled(false);
    iter = new QMapIterator<QString, int> (streetMap);
    int streetid;
    while(iter->hasNext())
    {
        qApp->processEvents();
        if(abortClicked)
            break;
        iter->next();
        int fn = iter->value();
        QString street = iter->key();
        if(street == "Umgehungsstraße")
            qDebug() << "debug halt";
        StreetInfo* curSti = nullptr;
        QString curStreet;
        StreetInfo* olderSti = nullptr;

        // see if any record of this street exists!
        if(fn > 0)
            streetid = streetsTableModel->findStreetId(SegmentDescription::updateToken(footnoteMap.value(fn)));
        else
            streetid = streetsTableModel->findStreetId(street);
        if(streetid > 0)
        {
            curSti = streetsTableModel->getStreetDef(streetid);
            curStreet = curSti->street;
            //StreetInfo* sti = streetsTableModel->getStreetName(street, "");
            StreetInfo* sti = nullptr;
            QList<StreetInfo*>myArray = streetsTableModel->getStreetName(street, "");
            if(!myArray.isEmpty())
            {
                foreach (StreetInfo* si, myArray) {
                    if(si->street == street && si->streetId == streetid && si->sequence > 0)
                    {
                       olderSti = si;

                        if(ui->dateEdit->date() < olderSti->dateStart)
                        {
                            ui->lblInfo->setText(QString("id %1 %2 change older startDate from %3 to %4")
                                            .arg(olderSti->streetId).arg(olderSti->street)
                                            .arg(olderSti->dateStart.toString("yyyy/MM/dd"))
                                                     .arg(ui->dateEdit->date().toString("yyyy/MM/dd")));
                            olderSti->dateStart =  ui->dateEdit->date();
                            streetsTableModel->updateStreetName(*olderSti);
                        }
                        else
                        {
                            if(ui->dateEdit->date() > olderSti->dateEnd)
                            {
                                ui->lblInfo->setText(QString("id %1 %2 change older endDate from %3 to %4")
                                .arg(olderSti->streetId).arg(olderSti->street)
                                    .arg(olderSti->dateEnd.toString("yyyy/MM/dd"))
                                             .arg(ui->dateEdit->date().toString("yyyy/MM/dd")));
                                olderSti->dateEnd = ui->dateEdit->date();
                                streetsTableModel->updateStreetName(*olderSti);
                            }

                            if(ui->dateEdit->date() < curSti->dateStart)
                            {
                                ui->lblInfo->setText(QString("id %1 %2 change current startDate from %3 to %4")
                                .arg(curSti->streetId).arg(curSti->street)
                                    .arg(curSti->dateStart.toString("yyyy/MM/dd"))
                                             .arg(ui->dateEdit->date().addDays(1).toString("yyyy/MM/dd")));
                                curSti->dateStart = ui->dateEdit->date().addDays(1);
                                streetsTableModel->updateStreetName(*curSti);
                            }
                        }
                    }
                }
            }
            else
            {
                StreetInfo sti = StreetInfo();
                sti.streetId = streetid;
                sti.street = street;
                sti.dateStart = ui->dateEdit->date();
                sti.dateEnd = ui->dateEdit->date().addDays(1);
                sti.newerName = curStreet;
                sti.sequence = 1;
                if(streetsTableModel->addOldStreetName(&sti))
                {
                    lookupStreet(ui->textEdit, abbrevMap.value(street));
                }
            }
        }
        else
        {
            // no record exists

            // must add a record for a current streetname
            curSti = new StreetInfo();
            curSti->street = fn==0?street:footnoteMap.value(fn);
            curSti->dateStart = ui->dateEdit->date().addDays(2);
            curSti->sequence =0;
            if(streetsTableModel->newStreetDef(curSti))
                streetid = curSti->streetId;
            lookupStreet(ui->textEdit, abbrevMap.value(street));

            if(fn>0)
            {
                olderSti = new StreetInfo();
                olderSti->street = street;
                olderSti->dateStart = ui->dateEdit->date().addDays(2);
                olderSti->sequence =1;
                olderSti->streetId = streetid;
                streetsTableModel->newStreetName(olderSti);
            }
        }


        QStringList names;
        names.append(street);
        if(!curStreet.isEmpty())
            names.append(curStreet);
        QList<SegmentInfo> list = streetsTableModel->getSegmentsForStreet(names);
        bool bSegmentNeedsUpdate;
        foreach (SegmentInfo si, list) {
            if(si.streetId() < 0)
                bSegmentNeedsUpdate = true;
        }
        if(ui->checkBox->isChecked())
        {
            if(streetid > 0  && (fn > 0  ) &&!bSegmentNeedsUpdate)
                continue;
        }
         if(fn == 0)
        {

            // QList<StreetInfo*>* namesList = streetsTableModel->getStreetNames(streetid, &names);
            // Q_UNUSED(namesList);
            //names.append(street);

            int ix;
            if(!list.isEmpty())
            {
                for(ix = 0; ix < list.count(); ix++) {
                    SegmentInfo si = list.at(ix);
                    ui->groupBox->setTitle(tr("Connections for %1 ").arg(street));
                    segmentViewProxyModel = new QSortFilterProxyModel();
                    segmentViewSourceModel = new SegmentViewTableModel();
                    segmentViewProxyModel->setSourceModel(segmentViewSourceModel);
                    ui->segmentView->setModel(segmentViewProxyModel);
                    ui->segmentView->setSortingEnabled(true);
                    //ui->segmentView->resizeColumnsToContents();
                    ui->segmentView->setWordWrap(true);
                    ui->segmentView->horizontalHeader()->setStretchLastSection(true);
                    ui->segmentView->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(ui->segmentView, &QTableView::customContextMenuRequested, this,[=](QPoint pt){
                        segmentViewContextMenu(pt);
                    });
                    if(config->dus.colWidths.isEmpty())
                    {
                        for(int i =0; i<segmentViewSourceModel->columnCount(QModelIndex());i++ )
                        {
                            config->dus.colWidths.append(ui->segmentView->columnWidth(i));
                        }
                    }
                    else
                    {
                        for(int i =0; i<segmentViewSourceModel->columnCount(QModelIndex());i++ )
                        {
                            ui->segmentView->setColumnWidth(i,config->dus.colWidths.at(i));
                        }
                    }

                    QMap<int, SegmentInfo> connections;
                    QList<SegmentInfo> connections1 = SQL::instance()->getIntersectingSegments(si.startLat(),si.startLon(),.020);
                    segmentViewSourceModel->checkList(connections1);
                    foreach(SegmentInfo si, connections1)
                    {
                        if(si.streetName() == street)
                            connections.insert(si.segmentId(),si);
                    }
                    QList<SegmentInfo> connections2 = SQL::instance()->getIntersectingSegments(si.endLat(),si.endLon(),.020);
                    segmentViewSourceModel->checkList(connections2);
                    foreach(SegmentInfo si, connections2)
                    {
                        if(si.streetName() == street)
                            connections.insert(si.segmentId(), si);
                    }
                    if(connections.isEmpty())
                        continue;
                    ui->groupBox->setVisible(true);

                    segmentViewSourceModel->setList(connections.values());
                    nextClicked = false;
                    buttonClicked = false;
                    while(!buttonClicked)
                    {
                        qApp->processEvents();
                    }
                    if(buttonClicked)
                    {
                        if(nextClicked)
                        {
                            ui->groupBox->setVisible(false);
                            break;
                        }
                        else if(prevClicked)
                        {
                            ui->groupBox->setVisible(false);
                            if(iter->hasPrevious())
                            {
                                iter->previous();
                                iter->previous();
                            }
                            break;

                        }
                        if(abortClicked)
                        {
                            ui->groupBox->setVisible(false);
                            ui->pbUpdate->setEnabled(true);
                            abortClicked = false;
                            break;
                        }
                        else
                            return;
                    }
                }
            }
            else
            {
                ui->groupBox->setVisible(false);
            }
            //}
        }
#if 0
        else // fn != 0
        {
            ui->lblInfo->clear();

            // street has a footnote!
            curStreet =SegmentDescription::updateToken( footnoteMap.value(fn)); // current street name
            curr_streetId = streetsTableModel->findStreetId(curStreet);
            if(curr_streetId < 0)
            {
                //curr_streetId = streetsTableModel->newStreetDef(curStreet, "", ui->dateEdit->date());
                StreetInfo sti = StreetInfo();
                sti.street = curStreet;
                sti.dateStart = ui->dateEdit->date().addDays(1);
                if(!streetsTableModel->newStreetDef(&sti))
                {
                    throw Exception();
                }
                curr_streetId = sti.streetId;
            }

            StreetInfo* curStreetSti = streetsTableModel->getStreetDef(curr_streetId);
            QStringList names;
            names.append(street);
            QList<SegmentInfo> segs = streetsTableModel->getSegmentsForStreet(names);
            foreach (SegmentInfo si, segs) {
                curStreetSti->updateBounds(si);
                if(!curStreetSti->segments.contains(si.segmentId()))
                    curStreetSti->segments.append(si.segmentId());
            }
            StreetInfo sti = StreetInfo();
            sti.streetId = curr_streetId;
            sti.street = street;
            sti.dateStart = ui->dateEdit->date();
            sti.dateEnd = QDate::fromString("2050/01/01", "yyyy/MM/dd");
            sti.newerName = curStreetSti->street;
            sti.sequence = 1;
            if(!streetsTableModel->addOldStreetName(&sti))
            {
                ui->lblHelp->setStyleSheet("color: magenta");
                ui->lblHelp->setText(tr("addOldStreetName did not add %1").arg(sti.street));
            }
            else
            {
                ui->lblInfo->setStyleSheet("color: blue");
                ui->lblInfo->setText(tr("addOldStreetName added %1").arg(sti.street));
            }

        }
#endif
        lookupStreet(ui->textEdit, abbrevMap.value(street));
        if(fn > 0)
        {
            lookupStreet(ui->textEdit_2, footnoteMap.value(fn));
        }
    }
    ui->pbUpdate->setEnabled(true);
    nextClicked = true;
    buttonClicked = true;
}

int DialogUpdateStreets::indexOfDigit(QString txt, int begin)
{
    QChar c;
    int len = txt.length();
    int ix = begin;
    while(ix < len)
    {
        c = txt.at(ix);
        if(c.isDigit())
            return ix;
        ix++;
    }
    return -1;
}
