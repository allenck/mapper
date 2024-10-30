#ifndef CCOMBOBOX_H
#define CCOMBOBOX_H
#include <QComboBox>


// subclass QComboBox to enable FocusOut signal
class CComboBox : public QComboBox
{
  Q_OBJECT
public:
    CComboBox(QWidget* parent)
        :QComboBox(parent)
    {
        //QWidget::setFocusPolicy(QWidget::StrongFocus);
    };

signals:
    void signalFocusOut(void);
protected:
    virtual void focusOutEvent(QFocusEvent*)
    {
        emit signalFocusOut();
    }
};
#endif // CCOMBOBOX_H
