#include "turncombo.h"

TurnCombo::TurnCombo(QWidget *parent) : QComboBox(parent)
{
 clear();
 addItem("Left", 1);
 addItem("Center", 0);
 addItem("Right",2);
}

TurnCombo::TurnCombo(QString center, QWidget *parent) : QComboBox(parent)
{
 clear();
 addItem("Left", 1);
 addItem(center, 0);
 addItem("Right",2);
}

void TurnCombo::setValue(int val)
{
 setCurrentIndex(findData(val));
}

int TurnCombo::getValue()
{
 return currentData().toInt();
}
