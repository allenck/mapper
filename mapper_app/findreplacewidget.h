#ifndef FINDREPLACEWIDGET_H
#define FINDREPLACEWIDGET_H

#include <QWidget>
#include <QTextDocument>

namespace Ui {
class FindReplaceWidget;
}

class QTextEdit;
class FindReplaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindReplaceWidget(QTextEdit *editor, QWidget *parent = nullptr);
    ~FindReplaceWidget();
    void setEditor(QTextEdit *editor);
    void show( bool b);
    void setTextSelection(QString s);

private:
    Ui::FindReplaceWidget *ui;
    QTextEdit* editor = nullptr;
    bool find(QString s, QTextDocument::FindFlags flags= QTextDocument::FindFlags());
    void replace(QString s);
    int curIndex = -1;
    QString selection;

protected:

};

#endif // FINDREPLACEWIDGET_H
