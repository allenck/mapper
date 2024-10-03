#ifndef MYMESSAGEBOX_H
#define MYMESSAGEBOX_H

#include <QMessageBox>
#include <QObject>

class MyMessageBox : public QMessageBox
{
 public:
  MyMessageBox(QWidget* parent = nullptr);
  MyMessageBox(QWidget* parent, QString title, QString msg, QMessageBox::Icon icon, QMessageBox::StandardButtons);

  static int warning(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons =QMessageBox::Ok);
  static int question(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons =QMessageBox::Ok);
  static int critical(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons =QMessageBox::Ok);
  static int info(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons =QMessageBox::Ok);

 private:
  void common();
  static int functionCommon(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons, QMessageBox::Icon);
};

#endif // MYMESSAGEBOX_H
