#include "htmltextedit.h"
#include <QFontDialog>
#include "configuration.h"
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QTextDocumentFragment>
#include <QColorDialog>
#include <QMimeData>

HtmlTextEdit::HtmlTextEdit(QWidget *parent) :
  QTextBrowser(parent)
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
 textZoomAct = new QAction(tr("Size +"), this);
 textZoomAct->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Plus));
 textZoomAct->setShortcutContext(Qt::WidgetShortcut);
 textUnzoomAct = new QAction(tr("Size -"), this);
 textUnzoomAct->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Minus));
 textUnzoomAct->setShortcutContext(Qt::WidgetShortcut);
 textZoom2xAct = new QAction(tr("Size x2"), this);
 textZoom2xAct->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Forward));
 textZoom2xAct->setShortcutContext(Qt::WidgetShortcut);
 textUnzoom2xAct = new QAction(tr("Size /2"), this);
 textUnzoom2xAct->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Back));
 textUnzoom2xAct->setShortcutContext(Qt::WidgetShortcut);
 setColorRedAct = new QAction(tr("Red"), this);
 setColorRedAct->setCheckable(true);
 setColorGreenAct = new QAction(tr("Green"),this);
 setColorGreenAct->setCheckable(true);
 setFontAct = new QAction(tr("Change Font..."),this);
 setColorBlackAct = new QAction(tr("Black"), this);
 setColorBlackAct->setCheckable(true);
 setColorBlueAct = new QAction(tr("Blue"),this);
 setColorBlueAct->setCheckable(true);
 setColorGrayAct = new QAction(tr("Gray"),this);
 setColorGrayAct->setCheckable(true);
 setTextColorAct = new QAction(tr("set text color"),this);
 setBackgroundColorAct = new QAction(tr("set background color"),this);
 pasteHtmlAct = new QAction(tr("Paste HTML"),this);
 pasteSaved = new QAction(tr("Paste saved"), this);
 copySaved = new QAction(tr("Copy to saved"), this);
 linkWebPageAct = new QAction(tr("Create link"),this);
 linkWebPageAct->setCheckable(true);
 insertHtmlFragmentAct = new QAction(tr("Paste Html fragment"),this);
 pasteLinkAct = new QAction(tr("Paste link"),this);
 h1Act = new QAction(tr("H1"), this);
 h1Act->setCheckable(true);
 setPlaceholderText("Enter text");

 connect(boldAction, SIGNAL(triggered(bool)), this, SLOT(OnBoldAction(bool)));
 connect(italicAction, SIGNAL(triggered(bool)), this, SLOT(OnItalicAction(bool)));
 connect(underlineAct, SIGNAL(triggered(bool)), this, SLOT(OnUnderlineAct(bool)));
 connect(textZoomAct, SIGNAL(triggered(bool)), this, SLOT(OnTextZoomAct()));
 connect(textUnzoomAct, SIGNAL(triggered(bool)), this, SLOT(OnTextUnzoomAct()));
 connect(textZoom2xAct, SIGNAL(triggered(bool)), this, SLOT(OnTextZoom2xAct()));
 connect(textUnzoom2xAct, SIGNAL(triggered(bool)), this, SLOT(OnTextUnzoom2xAct()));
 connect(setColorRedAct, SIGNAL(triggered(bool)), this , SLOT(OnSetColorRedAct(bool)));
 connect(setColorGreenAct, SIGNAL(triggered(bool)), this, SLOT(OnSetColorGreenAct(bool)));
 connect(setColorBlackAct, SIGNAL(triggered(bool)), this, SLOT(OnSetColorBlackAct(bool)));
 connect(setColorBlueAct, SIGNAL(triggered(bool)), this, SLOT(OnSetColorBlueAct(bool)));
 connect(setColorGrayAct, SIGNAL(triggered(bool)),this, SLOT(OnSetColorGrayAct(bool)));
 this->setContextMenuPolicy(Qt::CustomContextMenu);
 connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
 connect(setFontAct, SIGNAL(triggered()), this, SLOT(OnSetFontAct()));
 connect(pasteHtmlAct, SIGNAL(triggered()),this, SLOT(OnPasteHtmlAct()) );
 connect(pasteSaved, SIGNAL(triggered()), this, SLOT(OnPasteSaved()));
 connect(copySaved, SIGNAL(triggered()), this, SLOT(OnCopySaved()));
 connect(setBackgroundColorAct, SIGNAL(triggered(bool)), this, SLOT(OnSetBackgroundColor(bool)));
 connect(setTextColorAct, SIGNAL(triggered(bool)), this, SLOT(OnSetTextColor(bool)));
 connect(linkWebPageAct, &QAction::triggered, this,[=]{
     onLinkWebPage();
 });
 connect(insertHtmlFragmentAct, &QAction::triggered, this, [=]{
     onInsertHtmlFragment();
 });
 connect(pasteLinkAct, &QAction::triggered,this, [=]{
     onPasteLink();
 });
 connect(h1Act, SIGNAL(triggered(bool)),this, SLOT(OnH1Act()));

 setFontPointSize(9);
 setDirty(false);
}

void HtmlTextEdit::showContextMenu(QPoint pt)
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    QTextCursor cur = this->textCursor();
    QMenu *menu = this->createStandardContextMenu();
    QList<QAction*> acts = menu->actions();
    if(isHtmlFragment( clipboard->text())) // if html or html fragment, disallow regular paste.
    {
        foreach (QAction* act, acts) {
            if(act->objectName() == "edit-paste")
            {
                act->setEnabled(false);
                break;
            }
        }
    }
    if(cur.hasSelection())
    {
        menu->addSeparator();
        _cur = this->textCursor();
        _format = currentCharFormat();
        _pointsize = this->fontPointSize();
        _font = this->font();
        _textColor = this->textColor();

        menu->addAction(boldAction);
        menu->addAction(italicAction);
        menu->addAction(underlineAct);
        menu->addAction(h1Act);
        QMenu *colorMenu = new QMenu(tr("Color"));
        colorMenu->addAction(setColorRedAct);
        colorMenu->addAction(setColorGreenAct);
        colorMenu->addAction(setColorBlackAct);
        colorMenu->addAction(setColorBlueAct);
        colorMenu->addAction(setColorGrayAct);
        colorMenu->addAction(setTextColorAct);
        colorMenu->addAction(setBackgroundColorAct);
        menu->addMenu(colorMenu);
        menu->addSection(tr("Font: %1").arg(_font.family()));
        menu->addAction(setFontAct);
        menu->addSection(tr("Pointsize: %1").arg(_pointsize));
        QMenu* zoomMenu = new QMenu(tr("Zoom"));
        menu->addMenu(zoomMenu);
        zoomMenu->addAction(textZoomAct);
        zoomMenu->addAction(textZoom2xAct);
        zoomMenu->addAction(textUnzoomAct);
        zoomMenu->addAction(textUnzoom2xAct);
        if(mimeData->hasHtml())
            menu->addAction((pasteHtmlAct));
        if(isHtmlFragment(clipboard->text()))
            menu->addAction(insertHtmlFragmentAct);

        boldAction->setChecked(_format.fontWeight() == QFont::Bold);
        italicAction->setChecked(_format.fontItalic());
        underlineAct->setChecked(_format.fontUnderline());
        bool bAnchor = _format.isAnchor();
        QString hRef = _format.anchorHref();
        QStringList anchorNames = _format.anchorNames();
        linkWebPageAct->setChecked(bAnchor);
        if(_textColor == Qt::red)
            setColorRedAct->setChecked(true);
        if(_textColor == Qt::black)
            setColorBlackAct->setChecked(true);
        if(_textColor == Qt::blue)
            setColorBlueAct->setChecked(true);
        if(_textColor == Qt::green)
            setColorGreenAct->setChecked(true);
        if(_textColor == Qt::gray)
            setColorGrayAct->setChecked(true);

        menu->addAction(copySaved);
        menu->addAction(linkWebPageAct);
     }
     else
     {
         if(clipboard->text().startsWith("<!DOCTYPE HTML"))
            menu->addAction((pasteHtmlAct));
        else if(isHtmlFragment(clipboard->text()))
        {
            menu->addAction(insertHtmlFragmentAct);
        }
        else
        {
            if(clipboard->text().startsWith("https://") && clipboard->text().endsWith("/"))
            {
                menu->addAction(pasteLinkAct);
            }
        }
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
    if(checked)
        _format.setFontWeight(QFont::Bold);
    else
        _format.setFontWeight(QFont::Normal);
    _cur.mergeCharFormat(_format);
    setDirty(true);
}

void HtmlTextEdit::OnItalicAction(bool checked)
{
    _format.setFontItalic(checked);
    _cur.mergeCharFormat(_format);
    setDirty(true);
}

void HtmlTextEdit::OnUnderlineAct(bool checked)
{
    _format.setFontUnderline(checked);
    _cur.mergeCharFormat(_format);
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
    setColorRedAct->setChecked(false);
    setColorGreenAct->setChecked(false);
    setColorBlackAct->setChecked(false);
    setColorBlueAct->setChecked(false);
    setColorGrayAct->setChecked(false);

    if(this->textColor()== Qt::red)
        setColorRedAct->setChecked(true);
    if(this->textColor() == Qt::darkGreen)
        setColorGreenAct->setChecked(true);
    if(this->textColor()== Qt::black)
        setColorBlackAct->setChecked(true);
    if(this->textColor()== Qt::blue)
        setColorBlueAct->setChecked(true);
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
void HtmlTextEdit::OnTextZoom2xAct()
{
    double pointsize = this->fontPointSize();
    if(pointsize <= 0)
        this->setFontPointSize(11);
    this->setFontPointSize(this->fontPointSize()*2.);
    setDirty(true);
}
void HtmlTextEdit::OnTextUnzoomAct()
{
    this->setFontPointSize(this->fontPointSize()-1.);
    setDirty(true);
}
void HtmlTextEdit::OnTextUnzoom2xAct()
{
    this->setFontPointSize(this->fontPointSize()/2.);
    setDirty(true);
}

void HtmlTextEdit::OnSetColorRedAct(bool checked)
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
void HtmlTextEdit::OnSetColorBlueAct(bool checked)
{
    Q_UNUSED(checked)
    this->setTextColor(Qt::blue);
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
 font.setWeight(QFont().weight());
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
    QString clipText = clipboard->text();
    const QMimeData *mimeData = clipboard->mimeData();
    QString mimeText = mimeData->text();

     if (mimeData->hasHtml()) {
         //this->insertHtml(mimeData->html());
         this->insertFromMimeData(mimeData);
         //setTextFormat(Qt::RichText);
     } else if (mimeData->hasText()) {
         //this->insertPlainText(mimeData->text());
         // QTextCursor cursor =textCursor();
         // cursor.insertText(mimeData->text());
         this->clear();
         QString htmlText = this->toHtml();
         int ix = htmlText.indexOf("</body>");
         htmlText.insert(ix, mimeData->text());
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

void HtmlTextEdit::OnSetTextColor(bool checked)
{
    Q_UNUSED(checked)
    QColor c = QColorDialog::getColor(this->textColor());

    this->setTextColor(c);
    setDirty(true);
}

void HtmlTextEdit::OnSetBackgroundColor(bool)
{
 QColor c = QColorDialog::getColor(this->textBackgroundColor());
 setTextBackgroundColor(c);
}

void HtmlTextEdit::onLinkWebPage()
{
    QTextCursor cursor = textCursor();
    QString address = cursor.selectedText();
    QTextCharFormat fmt = cursor.charFormat();
    fmt.setForeground(QColor('blue'));
    fmt.setAnchor(true);
    fmt.setAnchorHref(address);
    fmt.setToolTip(address);
    cursor.insertText(address, fmt);
}

bool HtmlTextEdit::isHtmlFragment(QString text)
{
    if(text.contains("<p ",Qt::CaseInsensitive)) return true;
    if(text.contains("</p>>,",Qt::CaseInsensitive)) return true;
    if(text.contains("<a ",Qt::CaseInsensitive)) return true;
    if(text.contains("</a>,",Qt::CaseInsensitive)) return true;
    if(text.contains("</span>",Qt::CaseInsensitive)) return true;
    if(text.contains("<a ",Qt::CaseInsensitive)) return true;
    if(text.contains("<span >",Qt::CaseInsensitive)) return true;
    if(text.contains("<em>,",Qt::CaseInsensitive)) return true;
    if(text.contains("</em>,",Qt::CaseInsensitive)) return true;
    if(text.contains("<img >,",Qt::CaseInsensitive)) return true;
    if(text.contains("</strong>,",Qt::CaseInsensitive)) return true;
    if(text.contains("<strong>,",Qt::CaseInsensitive)) return true;
    if(text.contains("<blockquote >,",Qt::CaseInsensitive)) return true;
    if(text.contains("</script>,",Qt::CaseInsensitive)) return true;
    if(text.contains("<script >,",Qt::CaseInsensitive)) return true;
    if(text.contains("<br>,",Qt::CaseInsensitive)) return true;
    if(text.contains("</br>,",Qt::CaseInsensitive)) return true;

    return false;
}

void HtmlTextEdit::onInsertHtmlFragment()
{
    const QClipboard *clipboard = QApplication::clipboard();
    QString clipText = clipboard->text();

    QTextDocumentFragment frag = QTextDocumentFragment::fromHtml(clipText);
    this->textCursor().insertFragment(frag);
}

/*static*/ bool HtmlTextEdit::isLink(QString text)
{
    if(!text.startsWith("https://"))
        return false;
    QTextEdit* edit = new QTextEdit();
    edit->setHtml(text);
    QString html = edit->toHtml();
    QString search = "<a href = \"" +text + "\">";
    int ix = html.indexOf(search);
    if(ix > 0)
        return true;
    else
        return false;
}

void HtmlTextEdit::onPasteLink()
{
    const QClipboard *clipboard = QApplication::clipboard();

    QTextDocumentFragment frag = QTextDocumentFragment::fromHtml("<a href=" + clipboard->text() + "><span style=\" font-family:'Ubuntu'; text-decoration: underline; color:#6c7565;\">"
                                   + clipboard->text() + "</span></a></p>");
    this->textCursor().insertFragment(frag);
}

void HtmlTextEdit::OnH1Act(bool checked)
{
    QTextCursor cur = this->textCursor();
    double pointsize = this->fontPointSize();
    if(pointsize <= 0)
        this->setFontPointSize(11);
    if(checked)
    {
        this->setFontWeight(75);
        this->setFontPointSize(this->fontPointSize()+4.);
    }
    else
    {
        this->setFontWeight(50);
        this->setFontPointSize(this->fontPointSize()-4.);

    }
    h1Act->setChecked(!checked);

    setDirty(true);
}
