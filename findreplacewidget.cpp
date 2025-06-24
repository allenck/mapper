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
    connect(ui->btnFindNext, &QPushButton::clicked, this, [=]{
        QTextDocument::FindFlags flags= QTextDocument::FindFlags();
        if(ui->chkCaseSensitive->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if(ui->chkWhole->isChecked())
            flags |= QTextDocument::FindWholeWords;
        find(ui->txtFind->text(),flags);
    });
    connect(ui->btnReplace, &QPushButton::clicked, this,[=]{
        QTextCursor cursor = editor->textCursor();
        cursor.insertText(ui->txtReplace->text());
    });
    connect(ui->btnFindPrev, &QPushButton::clicked, this,[=]{
        QTextDocument::FindFlags flags= QTextDocument::FindFlags();
        if(ui->chkCaseSensitive->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if(ui->chkWhole->isChecked())
            flags |= QTextDocument::FindWholeWords;
        flags |= QTextDocument::FindBackward;
        find(ui->txtFind->text(),flags);

    });
    connect(ui->btnReplaceAndFind, &QPushButton::clicked, this,[=]{
        QTextCursor cursor = editor->textCursor();
        cursor.insertText(ui->txtReplace->text());

        QTextDocument::FindFlags flags= QTextDocument::FindFlags();
        if(ui->chkCaseSensitive->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if(ui->chkWhole->isChecked())
            flags |= QTextDocument::FindWholeWords;
        find(ui->txtFind->text(),flags);

    });
    connect(ui->btnReplaceAll, &QPushButton::clicked, this,[=]{
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
    connect(ui->btnSelectAll, &QPushButton::clicked, this,[=]{
        while(true)
        {
            QTextCursor cursor;
            cursor.setPosition(0);
            editor->setTextCursor(cursor);

            QTextDocument::FindFlags flags= QTextDocument::FindFlags();
            if(ui->chkCaseSensitive->isChecked())
                flags |= QTextDocument::FindCaseSensitively;
            if(ui->chkWhole->isChecked())
                flags |= QTextDocument::FindWholeWords;
            if(!find(ui->txtFind->text(),flags))
                break;

        }
    });

    connect(ui->btnHide, &QPushButton::clicked,this,[=]{
        this->setHidden(true);
    });
}

FindReplaceWidget::~FindReplaceWidget()
{
    delete ui;
}

bool FindReplaceWidget::find(QString s, QTextDocument::FindFlags flags)
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
    }
    ui->btnReplace->setEnabled(found);
    return found;
}

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
