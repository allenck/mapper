#include "rtitemdelegate.h"
#include "data.h"
#include "qcombobox.h"

RTItemDelegate::RTItemDelegate()
{
 //enum RouteType { Surface, SurfacePRW, RapidTransit, Subway, Rail, Incline, Other };

 routeTypes = {{RouteType::Surface, "Surface"},
               {RouteType::SurfacePRW, "Surface PRW"},
               {RouteType::RapidTransit, "Rapid Transit"},
               {RouteType::Subway, "Subway"},
               {RouteType::Rail, "Rail"},
               {RouteType::Incline, "Incline"},
               {RouteType::MagLev, "Maglev"},
               {RouteType::Elevated, "Elevated"},
               {RouteType::Other, "Other"}
              };
}

QWidget* RTItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const
{

 QComboBox* editor;

 editor = new QComboBox(parent);
 int ix=0;
 foreach(QString descr , routeTypes.values())
 {
  editor->addItem(descr, ix++);
 }
 return editor;
}

void RTItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
 QComboBox *comboBox = static_cast<QComboBox*>(editor);
 int value = index.model()->data(index, Qt::EditRole).toUInt();
 comboBox->setCurrentText(index.data().toString());
}

void RTItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
 QComboBox *comboBox = static_cast<QComboBox*>(editor);
 model->setData(index, comboBox->currentData().toInt(), Qt::EditRole);
}

