#include "findreplacewidget.h"
#include "qevent.h"
#include "ui_findreplacewidget.h"
#include <QTextEdit>
#include <QKeyEvent>

FindReplaceWidget::FindReplaceWidget(QTextEdit* editor,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindReplaceWidget)
{
    ui->setupUi(this);
    this->editor = editor;
    // QTextCursor cursor;
    // cursor.setPosition(1);
    // editor->setTextCursor(cursor);
    connect(ui->btnFindNext, &ClickableLabel::clicked, this, [=]{
        if(ui->txtFind->text().isEmpty())
            return;
        QTextDocument::FindFlags flags= QTextDocument::FindFlags();
        if(ui->chkCaseSensitive->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if(ui->chkWhole->isChecked())
            flags |= QTextDocument::FindWholeWords;
        if(!find(ui->txtFind->text(),flags))
        {
            QTextCursor cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::Start);
            editor->setTextCursor(cursor);
            find(ui->txtFind->text(),flags);
        }
    });
    connect(ui->btnReplace, &ClickableLabel::clicked, this,[=]{
        if(ui->txtFind->text().isEmpty())
            return;
        QTextCursor cursor = editor->textCursor();
        cursor.insertText(ui->txtReplace->text());
    });
    connect(ui->btnFindPrev, &ClickableLabel::clicked, this,[=]{
        if(ui->txtFind->text().isEmpty())
            return;
        QTextDocument::FindFlags flags= QTextDocument::FindFlags();
        if(ui->chkCaseSensitive->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if(ui->chkWhole->isChecked())
            flags |= QTextDocument::FindWholeWords;
        flags |= QTextDocument::FindBackward;
        if(!find(ui->txtFind->text(),flags))
        {
            QTextCursor cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::End);
            editor->setTextCursor(cursor);
            find(ui->txtFind->text(),flags);
        }

    });
    connect(ui->btnReplaceAndFind, &ClickableLabel::clicked, this,[=]{
        if(ui->txtFind->text().isEmpty())
            return;
        QTextCursor cursor = editor->textCursor();
        cursor.insertText(ui->txtReplace->text());

        QTextDocument::FindFlags flags= QTextDocument::FindFlags();
        if(ui->chkCaseSensitive->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if(ui->chkWhole->isChecked())
            flags |= QTextDocument::FindWholeWords;
        if(!find(ui->txtFind->text(),flags))
        {
            QTextCursor cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::Start);
            editor->setTextCursor(cursor);
            find(ui->txtFind->text(),flags);
        }

    });
    connect(ui->btnReplaceAll, &ClickableLabel::clicked, this,[=]{
        if(ui->txtFind->text().isEmpty())
            return;
        while(true)
        {
            QTextCursor cursor = editor->textCursor();
            cursor.insertText(ui->txtReplace->text());

            QTextDocument::FindFlags flags= QTextDocument::FindFlags();
            if(ui->chkCaseSensitive->isChecked())
                flags |= QTextDocument::FindCaseSensitively;
            if(ui->chkWhole->isChecked())
                flags |= QTextDocument::FindWholeWords;
            if(!find(ui->txtFind->text(),flags))
                break;
        }
    });
    connect(ui->btnSelectAll, &ClickableLabel::clicked, this,[=]{
        if(ui->txtFind->text().isEmpty())
            return;
        QTextCursor cursor;
        cursor.setPosition(0);
        editor->setTextCursor(cursor);
        while(true)
        {
            QTextDocument::FindFlags flags= QTextDocument::FindFlags();
            if(ui->chkCaseSensitive->isChecked())
                flags |= QTextDocument::FindCaseSensitively;
            if(ui->chkWhole->isChecked())
                flags |= QTextDocument::FindWholeWords;
            if(!find(ui->txtFind->text(),flags))
                return;
            cursor = editor->textCursor();
        }
    });

    connect(ui->btnHide, &ClickableLabel::clicked,this,[=]{
        this->setHidden(true);
    });

    connect(ui->txtFind, &QLineEdit::textEdited, this, [=](QString txt){
        highlightText(txt);
    });

    connect(ui->chkCaseSensitive, &QCheckBox::checkStateChanged,this,[=]{
        highlightText(ui->txtFind->text());
    });

    connect(ui->chkWhole, &QCheckBox::checkStateChanged,this,[=]{
        highlightText(ui->txtFind->text());
    });
}

FindReplaceWidget::~FindReplaceWidget()
{
    delete ui;
}

bool FindReplaceWidget::find(QString s, QTextDocument::FindFlags flags, int act)
{
    bool found = editor->find(s,flags);
    if(found)
    {
        QTextCursor cursor = editor->textCursor();
        curIndex = cursor.position();
        //cursor.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,2);
        cursor.setPosition(curIndex, QTextCursor::KeepAnchor);
        editor->setTextCursor(cursor);
        selection = cursor.selectedText();

        switch(act)
        {
            case 0:
                break;
            case 1:
            {
                QTextCharFormat backgrounder;
                backgrounder.setBackground(Qt::yellow);
                cursor.setCharFormat(backgrounder);
                break;
            }
            case 2:
            {
                QTextCharFormat backgrounder;
                backgrounder.setBackground(Qt::white);
                cursor.setCharFormat(backgrounder);
                break;
            }
        }
    }
    ui->btnReplace->setEnabled(found);
    return found;
}

void FindReplaceWidget::highlightText(QString txt)
{
    QTextCursor cursor;

    if(!allSelected.isEmpty())
    {
        cursor.setPosition(0);
        editor->setTextCursor(cursor);
        while(true)
        {
            // QTextDocument::FindFlags flags = QTextDocument::FindFlags();
            // if(ui->chkCaseSensitive->isChecked())
            //     flags |= QTextDocument::FindCaseSensitively;
            // if(ui->chkWhole->isChecked())
            //     flags |= QTextDocument::FindWholeWords;
            if(!find(allSelected,oldflags,2))
                break;
            cursor = editor->textCursor();
        }
    }

    cursor.setPosition(0);
    editor->setTextCursor(cursor);
    QTextDocument::FindFlags flags= QTextDocument::FindFlags();
    if(ui->chkCaseSensitive->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if(ui->chkWhole->isChecked())
        flags |= QTextDocument::FindWholeWords;
    oldflags = flags;
    while(true)
    {
        if(!find(txt,flags,1))
            break;
        cursor = editor->textCursor();
    }
    allSelected= txt;
};

void FindReplaceWidget::replace(QString s)
{
    QTextCursor cursor = editor->textCursor();
    cursor.insertText(s);
}

void FindReplaceWidget::setEditor(QTextEdit *editor)
{
    this->editor = editor;
}

void FindReplaceWidget::show( bool b)
{
    this->setVisible(b);
}

void FindReplaceWidget::setTextSelection(QString s)
{
    ui->txtFind->setText(s);
}
