#include "browsecommentsdialog.h"
#include "ui_browsecommentsdialog.h"
#include "htmltextedit.h"
#include "sql.h"


BrowseCommentsDialog::BrowseCommentsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::BrowseCommentsDialog)
{
 ui->setupUi(this);
 ui->txtComments->setReadOnly(false);
 connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(OnBtnNext()));
 connect(ui->btnPrev, SIGNAL(clicked()), this, SLOT(OnBtnPrev()));
 connect(ui->txtKey, &QLineEdit::editingFinished, [=]{
  bool bOk;
  int commentKey = ui->txtKey->text().toInt(&bOk);
  if(bOk)
  {
   ci = CommentInfo(commentKey-1);
   OnBtnNext();
  }
 });
}

BrowseCommentsDialog::~BrowseCommentsDialog()
{
 delete ui;
}

void BrowseCommentsDialog::btnDelete_Clicked()
{
    SQL::instance()->deleteComment(ci.commentKey);
    this->close();
}

void BrowseCommentsDialog::OnBtnNext()
{
    outputChanges();

    getComment(+1);

    ui->btnNext->setEnabled(ci.commentKey != -1);

}

void BrowseCommentsDialog::OnBtnPrev()
{
    outputChanges();

    getComment(-1);

    ui->btnPrev->setEnabled(ci.commentKey != -1);


}

void BrowseCommentsDialog::getComment(int pos)
{
 ci = SQL::instance()->getComment(ci.commentKey, pos);
 ui->txtComments->setHtml(ci.comments);
 ui->txtKey->setText(tr("%1").arg(ci.commentKey));
 //ui->txtUsedByS->setText(ci.usedByStations.join(","));
 ui->txtUsedByRoutes->setText(ci.usedByRoutes.join(","));

}

void BrowseCommentsDialog::outputChanges()
{
 ci.comments = ui->txtComments->toHtml();
 if(ci.commentKey >= 0 || !ci.comments.isEmpty())
  SQL::instance()->updateComment(ci.commentKey, ci.comments);
}
