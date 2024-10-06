#ifndef DIALOGTEXTEDIT_H
#define DIALOGTEXTEDIT_H

#include <QDialog>

namespace Ui {
 class DialogTextEdit;
}

class DialogTextEdit : public QDialog
{
  Q_OBJECT

 public:
  explicit DialogTextEdit(QString title, QString Label, QWidget *parent = nullptr);
  ~DialogTextEdit();
  QString result();

 private:
  Ui::DialogTextEdit *ui;
  QString title;
  QString label;
};

#endif // DIALOGTEXTEDIT_H
