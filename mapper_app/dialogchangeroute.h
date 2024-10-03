#ifndef DIALOGCHANGEROUTE_H
#define DIALOGCHANGEROUTE_H

#include <QDialog>

namespace Ui {
 class DialogChangeRoute;
}

class DialogChangeRoute : public QDialog
{
  Q_OBJECT

 public:
  explicit DialogChangeRoute(QWidget *parent = nullptr);
  ~DialogChangeRoute();
 int getNumber();
 private:
  Ui::DialogChangeRoute *ui;
  int number;
  int lowRange;
  int highRange;
};

#endif // DIALOGCHANGEROUTE_H
