#ifndef BROWSECOMMENTSDIALOG_H
#define BROWSECOMMENTSDIALOG_H

#include <QDialog>
#include "data.h"

namespace Ui {
 class BrowseCommentsDialog;
}

class BrowseCommentsDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit BrowseCommentsDialog(QWidget *parent = nullptr);
  ~BrowseCommentsDialog();

 public slots:
  void btnDelete_Clicked();
  void OnBtnNext();
  void OnBtnPrev();


 private:
  Ui::BrowseCommentsDialog *ui;
  void outputChanges();
  CommentInfo ci;
  void getComment(int pos);

};

#endif // BROWSECOMMENTSDIALOG_H
