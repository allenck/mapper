#include "dateeditdelegate.h"
#include "myapplication.h"
#include "qdatetimeedit.h"

DateEditDelegate::DateEditDelegate(QObject *parent)
    : QItemDelegate{parent}
{
    _dtFormat = ((MyApplication*)QApplication::instance())->dateFormat();

}

QWidget *DateEditDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &/* option */,
                                        const QModelIndex &/* index */) const
{
    QDateEdit * editor= new QDateEdit(parent);
    editor->setDisplayFormat(_dtFormat);

    return editor;
}

void DateEditDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::DisplayRole).toString();

    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    dateEdit->setDate(QDate::fromString(value, _dtFormat));
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
