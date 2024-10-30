#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>

class MyApplication : public QApplication
{
 public:
  MyApplication(int &argc, char *argv[]);

 private:
     bool notify(QObject *receiver_, QEvent *event_);

};

#endif // MYAPPLICATION_H
