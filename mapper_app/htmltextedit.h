#ifndef HTMLTEXTEDIT_H
#define HTMLTEXTEDIT_H

#include <QTextEdit>
#include <QAction>

class Configuration;
class HtmlTextEdit : public QTextEdit
{
 Q_OBJECT
public:
 explicit HtmlTextEdit(QWidget *parent = 0);

signals:
 void dirtySet(bool dirty);

public slots:

private:
 QAction *boldAction;
 QAction *italicAction;
 QAction *underlineAct;
 QAction *textZoomAct;
 QAction *textUnzoomAct;
 QAction *setColorAct;
 QAction *setColorGreenAct;
 QAction *setColorBlackAct;
 QAction *setColorGrayAct;
 QAction *setFontAct;
 QAction *pasteHtmlAct;
 QAction *pasteSaved;
 QAction *copySaved;
 bool bIsDirty;
 void setDirty(bool dirty);
 Configuration* config;

private slots:
 void showContextMenu(const QPoint &pt);
 void OnBoldAction(bool checked);
 void OnItalicAction(bool checked);
 void OnUnderlineAct(bool checked);
 void OnSelectionChanged();
 void OnTextZoomAct();
 void OnTextUnzoomAct();
 void OnSetColorAct(bool checked);
 void OnSetColorGreenAct(bool checked);
 void OnSetColorBlackAct(bool checked);
 void OnSetColorGrayAct(bool checked);
 void OnSetFontAct();
 void OnPasteHtmlAct();
 void OnPasteSaved();
 void OnCopySaved();
 void OnTextChanged();

};

#endif // HTMLTEXTEDIT_H
