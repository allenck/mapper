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
    ui->listWidget->addItems(list);
}

QString DialogSelectList::getResult()
{
    return result;
}