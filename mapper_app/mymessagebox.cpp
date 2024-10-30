#include "mymessagebox.h"

MyMessageBox::MyMessageBox(QWidget *parent)
{
 common();
}

MyMessageBox::MyMessageBox(QWidget* parent, QString title, QString msg, QMessageBox::Icon icon, QMessageBox::StandardButtons buttons)
{
 common();
 QMessageBox::setParent(parent);
 setWindowTitle(title);
 setText(msg);
 setIcon(icon);
 setStandardButtons(buttons);
}


void MyMessageBox::common()
{
 setIcon(QMessageBox::Information);
 setStyleSheet("background-color: #faf0e6;");
 setStyleSheet("text-color: rgb(0, 0, 0);");
 setStandardButtons(QMessageBox::Ok);
 setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Dialog);
 setModal(true);
}

/*static*/ int MyMessageBox::warning(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons)
{
 return functionCommon(parent, title,msg, buttons, QMessageBox::Warning);
}

/*static*/ int MyMessageBox::question(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons)
{
 return functionCommon(parent, title,msg, buttons, QMessageBox::Question);
}
/*static*/ int MyMessageBox::critical(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons)
{
 return functionCommon(parent, title,msg, buttons, QMessageBox::Critical);

}


int MyMessageBox::functionCommon(QWidget* parent, QString title, QString msg, QMessageBox::StandardButtons buttons, QMessageBox::Icon icon)
{
 MyMessageBox* mb = new MyMessageBox(parent, title, msg, icon, buttons);
 return mb->exec();
}
