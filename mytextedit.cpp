#include "mytextedit.h"
#include <QFontDialog>
#include <QColorDialog>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QAction>

MyTextEdit::MyTextEdit(QWidget* parent) : QTextEdit(parent)
{
 boldAction = new QAction(tr("Bold"));
 italicAction = new QAction(tr("Italicize"), this);
 underlineAct = new QAction(tr("Underline"),this);
 textZoomAct = new QAction(tr("Zoom +"), this);
 textZoomAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Plus));
 textZoomAct->setShortcutContext(Qt::WidgetShortcut);
 textUnzoomAct = new QAction(tr("Zoom -"), this);
 textUnzoomAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Minus));
 textUnzoomAct->setShortcutContext(Qt::WidgetShortcut);
 setFontAct = new QAction(tr("Change Font..."),this);
 setTextColorAct = new QAction(tr("text color"),this);
 setBackgroundColorAct = new QAction(tr("background color"),this);
 pasteHtmlAct = new QAction(tr("Paste HTML"),this);
 connect(boldAction, SIGNAL(triggered(bool)), this, SLOT(OnBoldAction(bool)));
 connect(italicAction, SIGNAL(triggered(bool)), this, SLOT(OnItalicAction(bool)));
 connect(underlineAct, SIGNAL(triggered(bool)), this, SLOT(OnUnderlineAct(bool)));
 connect(textZoomAct, SIGNAL(triggered(bool)), this, SLOT(OnTextZoomAct()));
 connect(textUnzoomAct, SIGNAL(triggered(bool)), this, SLOT(OnTextUnzoomAct()));
 connect(setFontAct, SIGNAL(triggered()), this, SLOT(OnSetFontAct()));
 connect(setBackgroundColorAct, SIGNAL(triggered(bool)), this, SLOT(OnSetTextBackgroundColor(bool)));
 connect(setTextColorAct, SIGNAL(triggered(bool)), this, SLOT(OnSetTextColor(bool)));
 connect(pasteHtmlAct, SIGNAL(triggered()),this, SLOT(OnPasteHtmlAct()) );
 this->setContextMenuPolicy(Qt::CustomContextMenu);
 connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
 connect(this, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
}

void MyTextEdit::showContextMenu(QPoint pt)
{
 const QClipboard *clipboard = QApplication::clipboard();
 const QMimeData *mimeData = clipboard->mimeData();

 QMenu *menu = createStandardContextMenu();
 menu->addAction(boldAction);
 boldAction->setCheckable(true);
 italicAction->setCheckable(true);
 menu->addAction(italicAction);
 underlineAct->setCheckable(true);
 menu->addAction(underlineAct);
 menu->addAction(setFontAct);
 menu->addAction(textZoomAct);
 menu->addAction(textUnzoomAct);
 menu->addAction(setTextColorAct);
 menu->addAction(setBackgroundColorAct);
 if(mimeData->hasHtml())
     menu->addAction((pasteHtmlAct));
 menu->exec(this->mapToGlobal(pt));
 delete menu;
}

void MyTextEdit::OnBoldAction(bool checked)
{
    QTextCursor cur = textCursor();
    if(checked)
        setFontWeight(75);
    else
        setFontWeight(50);
    boldAction->setChecked(!checked);
}

void MyTextEdit::OnItalicAction(bool checked)
{
    QTextCursor cur = textCursor();
    setFontItalic(!checked);
    italicAction->setChecked(!checked);
}

void MyTextEdit::OnUnderlineAct(bool checked)
{
    setFontUnderline(!checked);
    underlineAct->setChecked(!checked);
}

void MyTextEdit::OnTextZoomAct()
{
    double pointsize = this->fontPointSize();
    if(pointsize <= 0)
        QTextEdit:this->setFontPointSize(11);
    this->setFontPointSize(this->fontPointSize()+1.);
    setDirty(true);
}
void MyTextEdit::OnTextUnzoomAct()
{
    this->setFontPointSize(this->fontPointSize()-1.);
    setDirty(true);
}

void MyTextEdit::OnSetFontAct()
{
 QFontDialog fontDlg;
 //fontDlg.setFont(this->currentFont());
 QFont font = this->currentFont();
 font.setItalic(this->fontItalic());
 font.setWeight((QFont::Weight)this->fontWeight());
 font.setUnderline(this->fontUnderline());
 font.setPointSize(this->fontPointSize());
 bool ok;
 font = QFontDialog::getFont(&ok,font, this);
 if (ok)
 {
      // font is set to the font the user selected
      this->setCurrentFont(font);
      setDirty(true);
 }
 else
 {
  // the user canceled the dialog; font is set to the default
  // application font, QApplication::font()
 }
}
void MyTextEdit::OnTextChanged()
{
 setDirty(true);
}

void MyTextEdit::OnSetTextColor(bool checked)
{
    Q_UNUSED(checked)
    QColor c = QColorDialog::getColor(this->textColor());

    this->setTextColor(c);
    setDirty(true);
}

void MyTextEdit::OnSetTextBackgroundColor(bool)
{
 QColor c = QColorDialog::getColor(this->textBackgroundColor());
 setTextBackgroundColor(c);
 setDirty(true);
}

void MyTextEdit::OnPasteHtmlAct()
{
    const QClipboard *clipboard = QApplication::clipboard();
    QString clipText = clipboard->text();
    const QMimeData *mimeData = clipboard->mimeData();
    QString mimeText = mimeData->text();

     if (mimeData->hasHtml()) {
         this->setHtml(mimeData->html());
         //setTextFormat(Qt::RichText);
     } else if (mimeData->hasText()) {
         this->setText(mimeData->text());
         //setTextFormat(Qt::PlainText);
     }
//     else {
//         setText(tr("Cannot display data"));
}
