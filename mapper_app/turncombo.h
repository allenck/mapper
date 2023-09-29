#ifndef TURNCOMBO_H
#define TURNCOMBO_H

#include <QComboBox>

class TurnCombo : public QComboBox
{
 public:
  TurnCombo(QWidget* parent = nullptr);
  TurnCombo(QString center, QWidget* parent = nullptr);
  void setValue(int val);
  int getValue();
};

#endif // TURNCOMBO_H
