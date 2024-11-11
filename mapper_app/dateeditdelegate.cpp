#include "dateeditdelegate.h"
#include "qdatetimeedit.h"

DateEditDelegate::DateEditDelegate(QObject *parent)
    : QItemDelegate{parent}
{}

QWidget *DateEditDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &/* option */,
                                        const QModelIndex &/* index */) const
{
    QDateEdit * editor= new QDateEdit(parent);
    editor->setDisplayFormat("yyyy/MM/dd");

    return editor;
}

void DateEditDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::DisplayRole).toString();

    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    dateEdit->setDate(QDate::fromString(value, "yyyy/MM/dd"));
}

void DateEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    //    lineEdit->returnPressed();
    QString value = dateEdit->text();

    model->setData(index, value, Qt::EditRole);
}

void DateEditDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
