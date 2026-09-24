#include "dialogselectlist.h"
#include "qpushbutton.h"
#include "ui_dialogselectlist.h"

DialogSelectList::DialogSelectList(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogSelectList)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(ui->listWidget, &QAbstractItemView::clicked, this, [=](QModelIndex index){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        result = index.data().toString();
        if(!this->list.isEmpty())
            accept();
    });

}
DialogSelectList::~DialogSelectList()
{
    delete ui;
}

DialogSelectList::DialogSelectList(QStringList list, QWidget *parent) : QDialog(parent)
{
    setList(list);
    result.clear();
}

void DialogSelectList::setList(QStringList list)
{
    this->list = list;
    this->checkList.clear();
    ui->listWidget->clear();
    ui->listWidget->addItems(list);
}

// add list of checkable items
void DialogSelectList::setCheckList(QList<QPair<QString, bool>> items)
{
    this->checkList = items;
    ui->listWidget->clear();
    this->list.clear();
    ui->listWidget->blockSignals(true);
    for (const QPair<QString, bool> &pair : checkList) {
        QListWidgetItem *item = new QListWidgetItem(pair.first, ui->listWidget);

        // 1. Grant checkable flags
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        // 2. Set default check state so the box displays
        item->setCheckState(pair.second?Qt::Checked:Qt::Unchecked);

        ui->listWidget->addItem(item);
    }
    ui->listWidget->blockSignals(false);
    connect(ui->listWidget, &QListWidget::itemChanged, this, &DialogSelectList::onItemChanged);

}

void DialogSelectList::onItemChanged(QListWidgetItem *item)
{
    int rowIndex = ui->listWidget->row(item);
    QPair<QString,bool> pair = checkList.at(rowIndex);
    pair.second = item->checkState();
    checkList.replace(rowIndex,pair);
}

QString DialogSelectList::getResult()
{
    return result;
}

QList<QPair<QString, bool>> DialogSelectList::getCheckList()
{
    return checkList;
}

void DialogSelectList::setInstructions(QString text)
{
    ui->lblInstruction->setText(text);
}