#include <QtGui>
#include "editcomments.h"
#include "configuration.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMenuBar>
#include <QMenu>
#include <QGroupBox>
#include <QPushButton>

EditComments::EditComments()
{
    createMenu();
    createHorizontalGroupBox();
    txtEdit = new QTextEdit;
    txtTags = new QLineEdit;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar((QWidget*)menuBar);
    mainLayout->addWidget((QWidget*)horizontalGroupBox);
    mainLayout->addWidget(txtEdit);
    mainLayout->addWidget(txtTags);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    connect(txtEdit, SIGNAL(selectionChanged()), this, SLOT(OnSelectionChanged()));


    setWindowTitle(tr("Edit Comments"));
}
void EditComments::createMenu()
{
    menuBar = new QMenuBar();
    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));

    formatMenu = new QMenu(tr("Format"), this);
    boldAction = formatMenu->addAction(tr("Bold"));
    boldAction->setCheckable(true);
    italicAction =formatMenu->addAction(tr("Italicize"));
    italicAction->setCheckable(true);
    underlineAct = formatMenu->addAction(tr("Underline"));
    underlineAct->setCheckable(true);
    menuBar->addMenu(formatMenu);
    connect(boldAction, SIGNAL(triggered(bool)), this, SLOT(OnBoldAction(bool)));
    connect(italicAction, SIGNAL(triggered(bool)), this, SLOT(OnItalicAction(bool)));
    connect(underlineAct, SIGNAL(triggered(bool)), this, SLOT(OnUnderlineAct(bool)));


}
void EditComments::createHorizontalGroupBox()
 {
     horizontalGroupBox = new QGroupBox(tr("Horizontal layout"));
     QHBoxLayout *layout = new QHBoxLayout;

     for (int i = 0; i < 3; ++i) {
         buttons[i] = new QPushButton(tr("Button %1").arg(i + 1));
         layout->addWidget(buttons[i]);
     }
     horizontalGroupBox->setLayout(layout);
 }
void EditComments::setConfiguration(Configuration *cfg)
{
    config = cfg;
}
void EditComments::setHTMLText(QString html)
{
    txtEdit->setHtml(html);
}
void EditComments::setTags(QString tags)
{
    txtTags->setText( tags);
}
QString EditComments::HTML()
{
    QString str = txtEdit->toHtml();
    //qint32 count = str.length();
    return str;
}
QString EditComments::Tags()
{
    return txtTags->text();
}

void EditComments::OnBoldAction(bool checked)
{
    QTextCursor cur = txtEdit->textCursor();
    if(checked)
        txtEdit->setFontWeight(75);
    else
        txtEdit->setFontWeight(50);
    boldAction->setChecked(!checked);
}
void EditComments::OnItalicAction(bool checked)
{
    QTextCursor cur = txtEdit->textCursor();
    txtEdit->setFontItalic(!checked);
    italicAction->setChecked(!checked);
}
void EditComments::OnUnderlineAct(bool checked)
{
    txtEdit->setFontUnderline(!checked);
    underlineAct->setChecked(!checked);
}
void EditComments::OnSelectionChanged()
{
    if(txtEdit->fontWeight()< 75)
        boldAction->setChecked(false);
    else
        boldAction->setChecked(true);
    italicAction->setChecked(txtEdit->fontItalic());
    underlineAct->setChecked(txtEdit->fontUnderline());
}
