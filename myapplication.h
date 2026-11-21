#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>

class MyApplication : public QApplication
{
 public:
  MyApplication(int &argc, char *argv[]);
     QString dateFormat() {return _dtFormat;}
     void setDateFormat(QString dateFormat){_dtFormat = dateFormat;}

 private:
     bool notify(QObject *receiver_, QEvent *event_);
     QString _dtFormat;

};

#endif // MYAPPLICATION_H
