#ifndef DIALOGEDITPARAMETERS_H
#define DIALOGEDITPARAMETERS_H

#include "data.h"
#include <QDialog>

namespace Ui {
 class DialogEditParameters;
}

class DialogEditParameters : public QDialog
{
  Q_OBJECT

 public:
  explicit DialogEditParameters(QWidget *parent = nullptr);
  ~DialogEditParameters();
  Parameters parameters() {return parms;}

 public slots:
  void btnOk();

 private:
  Ui::DialogEditParameters *ui;
  Parameters parms;
};

#endif // DIALOGEDITPARAMETERS_H
