#include "htmltextedit.h"
#include <QFontDialog>
#include "configuration.h"
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QTextDocumentFragment>

HtmlTextEdit::HtmlTextEdit(QWidget *parent) :
  QTextEdit(parent)
{
 config = Configuration::instance();
 connect(this, SIGNAL(selectionChanged()), this, SLOT(OnSelectionChanged()));
 connect(this, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
 boldAction = new QAction(tr("Bold"),this);
 boldAction->setCheckable(true);
 italicAction =new QAction(tr("Italicize"), this);
 italicAction->setCheckable(true);
 underlineAct = new QAction(tr("Underline"), this );
 underlineAct->setCheckable(true);
 textZoomAct = new QAction(tr("Zoom +"), this);
 textZoomAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Plus));
 textZoomAct->setShortcutContext(Qt::WidgetShortcut);
 textUnzoomAct = new QAction(tr("Zoom -"), this);
 textUnzoomAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Minus));
 textUnzoomAct->setShortcutContext(Qt::WidgetShortcut);
 setColorAct = new QAction(tr("Red"), this);
 setColorAct->setCheckable(true);
 setColorGreenAct = new QAction(tr("Green"),this);
 setColorGreenAct->setCheckable(true);
 setFontAct = new QAction(tr("Change Font..."),this);
 setColorBlackAct = new QAction(tr("Black"), this);
 setColorGrayAct = new QAction(tr("Gray"),this);
 setColorGrayAct->setCheckable(true);
 pasteHtmlAct = new QAction(tr("Paste HTML"),this);
 pasteSaved = new QAction(tr("Paste saved"), this);
 copySaved = new QAction(tr("Copy to saved"), this);

 connect(boldAction, SIGNAL(triggered(bool)), this, SLOT(OnBoldAction(bool)));
 connect(italicAction, SIGNAL(triggered(bool)), this, SLOT(OnItalicAction(bool)));
 connect(underlineAct, SIGNAL(triggered(bool)), this, SLOT(OnUnderlineAct(bool)));
 connect(textZoomAct, SIGNAL(triggered(bool)), this, SLOT(OnTextZoomAct()));
 connect(textUnzoomAct, SIGNAL(triggered(bool)), this, SLOT(OnTextUnzoomAct()));
 connect(setColorAct, SIGNAL(triggered(bool)), this , SLOT(OnSetColorAct(bool)));
 connect(setColorGreenAct, SIGNAL(triggered(bool)), this, SLOT(OnSetColorGreenAct(bool)));
 connect(setColorBlackAct, SIGNAL(triggered(bool)), this, SLOT(OnSetColorBlackAct(bool)));
 connect(setColorGrayAct, SIGNAL(triggered(bool)),this, SLOT(OnSetColorGrayAct(bool)));
 this->setContextMenuPolicy(Qt::CustomContextMenu);
 connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(const QPoint &)));
 connect(setFontAct, SIGNAL(triggered()), this, SLOT(OnSetFontAct()));
 connect(pasteHtmlAct, SIGNAL(triggered()),this, SLOT(OnPasteHtmlAct()) );
 connect(pasteSaved, SIGNAL(triggered()), this, SLOT(OnPasteSaved()));
 connect(copySaved, SIGNAL(triggered()), this, SLOT(OnCopySaved()));

 setFontPointSize(9);
 setDirty(false);
}

void HtmlTextEdit::showContextMenu(const QPoint &pt)
{
 const QClipboard *clipboard = QApplication::clipboard();
 const QMimeData *mimeData = clipboard->mimeData();

 QTextCursor cur = this->textCursor();
 QMenu *menu = this->createStandardContextMenu();
 if(cur.hasSelection())
 {
  QString anchor = this->anchorAt(pt);
  menu->addAction(boldAction);
  menu->addAction(italicAction);
  menu->addAction(underlineAct);
  QMenu *colorMenu = new QMenu(tr("Color"));
  colorMenu->addAction(setColorAct);
  colorMenu->addAction(setColorGreenAct);
  colorMenu->addAction(setColorBlackAct);
  colorMenu->addAction(setColorGrayAct);
  menu->addMenu(colorMenu);
  menu->addAction(setFontAct);
  menu->addAction(textZoomAct);
  menu->addAction(textUnzoomAct);
  if(mimeData->hasHtml())
      menu->addAction((pasteHtmlAct));

  if(this->fontWeight() == 75)
      boldAction->setChecked(true);
  else
      boldAction->setChecked(false);

  italicAction->setChecked(this->fontItalic());

  menu->addAction(copySaved);
 }
 if(config->currCity->savedClipboard.length()>0)
 {
  menu->addAction(pasteSaved);
 }

 menu->exec(this->mapToGlobal(pt));
 delete menu;
}
void HtmlTextEdit::OnBoldAction(bool checked)
{
    QTextCursor cur = this->textCursor();
    if(checked)
        this->setFontWeight(75);
    else
        this->setFontWeight(50);
    boldAction->setChecked(!checked);
    setDirty(true);
}
void HtmlTextEdit::OnItalicAction(bool checked)
{
    QTextCursor cur = this->textCursor();
    this->setFontItalic(checked);
    italicAction->setChecked(!checked);
    setDirty(true);
}
void HtmlTextEdit::OnUnderlineAct(bool checked)
{
    this->setFontUnderline(!checked);
    underlineAct->setChecked(!checked);
    setDirty(true);
}
void HtmlTextEdit::OnSelectionChanged()
{
    if(this->fontWeight()< 75)
        boldAction->setChecked(false);
    else
        boldAction->setChecked(true);
//    if(this->textColor() == QColor(0,0,0,255))
//        setColorAct->setChecked(false);
//    else
//        setColorAct->setChecked(true);
    setColorAct->setChecked(false);
    setColorGreenAct->setChecked(false);
    setColorBlackAct->setChecked(false);
    setColorGrayAct->setChecked(false);

    if(this->textColor()== Qt::red)
        setColorAct->setChecked(true);
    if(this->textColor() == Qt::darkGreen)
        setColorGreenAct->setChecked(true);
    if(this->textColor()== Qt::black)
        setColorBlackAct->setChecked(true);
    if(this->textColor()== Qt::gray)
        setColorGrayAct->setChecked(true);


    italicAction->setChecked(this->fontItalic());
    underlineAct->setChecked(this->fontUnderline());
}

void HtmlTextEdit::OnTextZoomAct()
{
    double pointsize = this->fontPointSize();
    if(pointsize <= 0)
        this->setFontPointSize(11);
    this->setFontPointSize(this->fontPointSize()+1.);
    setDirty(true);
}
void HtmlTextEdit::OnTextUnzoomAct()
{
    this->setFontPointSize(this->fontPointSize()-1.);
    setDirty(true);
}
void HtmlTextEdit::OnSetColorAct(bool checked)
{
    Q_UNUSED(checked)
    //QColor red = new QColor(1,0,0,255);
    //QColor black = new QColor(0,0,0,255);

    this->setTextColor(Qt::red);
    setDirty(true);
}
void HtmlTextEdit::OnSetColorGreenAct(bool checked)
{
    Q_UNUSED(checked)
    //QColor red = new QColor(1,0,0,255);
    //QColor black = new QColor(0,0,0,255);

    this->setTextColor(Qt::darkGreen);
    setDirty(true);
}
void HtmlTextEdit::OnSetColorBlackAct(bool checked)
{
    Q_UNUSED(checked)
    //QColor red = new QColor(1,0,0,255);
    //QColor black = new QColor(0,0,0,255);

    this->setTextColor(Qt::black);
    setDirty(true);
}
void HtmlTextEdit::OnSetColorGrayAct(bool checked)
{
    Q_UNUSED(checked)
    this->setTextColor(Qt::gray);
    setDirty(true);
}

void HtmlTextEdit::setDirty(bool dirty)
{
 bIsDirty = dirty;
 emit dirtySet(dirty);
}

void HtmlTextEdit::OnTextChanged()
{
 setDirty(true);
}


void HtmlTextEdit::OnSetFontAct()
{
 QFontDialog fontDlg;
 //fontDlg.setFont(this->currentFont());
 QFont font = this->currentFont();
 font.setItalic(this->fontItalic());
 font.setWeight(this->fontWeight());
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
void HtmlTextEdit::OnPasteHtmlAct()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
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
void HtmlTextEdit::OnPasteSaved()
{
    this->setHtml(config->currCity->savedClipboard);
}
void HtmlTextEdit::OnCopySaved()
{
 config->currCity->savedClipboard = this->textCursor().selection().toHtml();
}

