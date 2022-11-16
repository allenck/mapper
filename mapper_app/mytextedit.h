#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>
#include <QMenu>

class MyTextEdit : public QTextEdit
{
  Q_OBJECT
 public:
  MyTextEdit(QWidget *parent);

 public slots:
  void OnBoldAction(bool checked);
  void OnItalicAction(bool checked);
  void OnUnderlineAct(bool checked);
  void OnTextZoomAct();
  void OnTextUnzoomAct();
  void OnSetFontAct();
  void OnTextChanged();
  void showContextMenu(QPoint pt);
  void OnSetTextColor(bool checked);
  void OnSetTextBackgroundColor(bool checked);
  void OnPasteHtmlAct();

 private:
  QAction* boldAction;
  QAction* italicAction;
  QAction* underlineAct;
  QAction *textZoomAct;
  QAction *textUnzoomAct;
  QAction *setFontAct;
  QAction *setBackgroundColorAct;
  QAction *setTextColorAct;
  QAction* pasteHtmlAct;
  void setDirty(bool dirty){_dirty = dirty;}
  bool _dirty=false;
};

#endif // MYTEXTEDIT_H
