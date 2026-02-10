#include "dialoghtmledit.h"
#include "ui_dialoghtmledit.h"

DialogHtmlEdit::DialogHtmlEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogHtmlEdit)
{
    ui->setupUi(this);
    config = Configuration::instance();
    connect(ui->htmlText, &QPlainTextEdit::textChanged, this, [=]{
        if(!textChangeing)
        {
            textChangeing = true;
            htmlText = ui->htmlText->toPlainText();
            ui->textEdit->setHtml(htmlText);
            textChangeing=false;
        }
    });
    connect(ui->textEdit, &QTextEdit::textChanged, this, [=]{
        if(!textChangeing)
        {
            textChangeing = true;
            htmlText = ui->textEdit->toHtml();
            ui->htmlText->setPlainText(htmlText);
            textChangeing=false;
        }
    });
}

DialogHtmlEdit::~DialogHtmlEdit()
{
    delete ui;
}

void DialogHtmlEdit::setData(QString text)
{
    ui->htmlText->setPlainText(text);
    htmlText = text;
    ui->textEdit->setHtml(text);
}

QString DialogHtmlEdit::getSource()
{
    return ui->htmlText->toPlainText();
}
