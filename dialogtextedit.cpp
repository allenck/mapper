#include "dialogtextedit.h"
#include "ui_dialogtextedit.h"

DialogTextEdit::DialogTextEdit(QString title, QString label, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogTextEdit)
{
 ui->setupUi(this);
 this->title = title;
 setWindowTitle(title);
 this->label = label;
 ui->label->setText(label);
}

DialogTextEdit::~DialogTextEdit()
{
 delete ui;
}

QString DialogTextEdit::result() {
 return ui->lineEdit->text();
}
