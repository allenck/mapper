#ifndef HTMLTEXTEDIT_H
#define HTMLTEXTEDIT_H

#include <QTextEdit>
#include <QTextBrowser>
#include <QAction>

class Configuration;
class HtmlTextEdit : public QTextBrowser
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
 QAction *setColorRedAct;
 QAction *setColorGreenAct;
 QAction *setColorBlackAct;
 QAction *setColorGrayAct;
 QAction *setFontAct;
 QAction *pasteHtmlAct;
 QAction *pasteSaved;
 QAction *copySaved;
 QAction* setTextColorAct;
 QAction *setBackgroundColorAct;
 QAction* linkWebPageAct;
 bool bIsDirty;
 void setDirty(bool dirty);
 Configuration* config;

private slots:
 void showContextMenu(QPoint pt);
 void OnBoldAction(bool checked);
 void OnItalicAction(bool checked);
 void OnUnderlineAct(bool checked);
 void OnSelectionChanged();
 void OnTextZoomAct();
 void OnTextUnzoomAct();
 void OnSetColorRedAct(bool checked);
 void OnSetColorGreenAct(bool checked);
 void OnSetColorBlackAct(bool checked);
 void OnSetColorGrayAct(bool checked);
 void OnSetFontAct();
 void OnPasteHtmlAct();
 void OnPasteSaved();
 void OnCopySaved();
 void OnTextChanged();
 void OnSetTextColor(bool);
 void OnSetBackgroundColor(bool);
 void onLinkWebPage();
};

#endif // HTMLTEXTEDIT_H
