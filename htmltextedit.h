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
    static bool isHtmlFragment(QString text);
 static bool isLink(QString text);

signals:
 void dirtySet(bool dirty);

public slots:

private:
 QAction *boldAction;
 QAction *italicAction;
 QAction *underlineAct;
 QAction *textZoomAct;
 QAction *textUnzoomAct;
 QAction *textZoom2xAct;
 QAction *textUnzoom2xAct;
 QAction *setColorRedAct;
 QAction *setColorGreenAct;
 QAction *setColorBlackAct;
 QAction *setColorBlueAct;
 QAction *setColorGrayAct;
 QAction *setFontAct;
 QAction *pasteHtmlAct;
 QAction *pasteSaved;
 QAction *copySaved;
 QAction* setTextColorAct;
 QAction *setBackgroundColorAct;
 QAction* linkWebPageAct;
 QAction* insertHtmlFragmentAct;
 QAction* pasteLinkAct;
 QAction* h1Act;
 // QAction* h2Act;
 // QAction* h3Act;
 bool bIsDirty;
 void setDirty(bool dirty);
 Configuration* config;
 QTextCursor _cur;
 QTextCharFormat _format;
 double _pointsize;
 QFont _font;
 QColor _textColor;

private slots:
 void showContextMenu(QPoint pt);
 void OnBoldAction(bool checked);
 void OnItalicAction(bool checked);
 void OnUnderlineAct(bool checked);
 void OnSelectionChanged();
 void OnTextZoomAct();
 void OnTextUnzoomAct();
 void OnTextZoom2xAct();
 void OnTextUnzoom2xAct();
 void OnSetColorRedAct(bool checked);
 void OnSetColorGreenAct(bool checked);
 void OnSetColorBlackAct(bool checked);
 void OnSetColorBlueAct(bool checked);
 void OnSetColorGrayAct(bool checked);
 void OnSetFontAct();
 void OnPasteHtmlAct();
 void OnPasteSaved();
 void OnCopySaved();
 void OnTextChanged();
 void OnSetTextColor(bool);
 void OnSetBackgroundColor(bool);
 void onLinkWebPage();
 void onInsertHtmlFragment();
 void onPasteLink();
 void OnH1Act(bool);
};

#endif // HTMLTEXTEDIT_H
