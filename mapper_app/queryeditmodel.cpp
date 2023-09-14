#include "queryeditmodel.h"

QueryEditModel::QueryEditModel(QObject *parent, QSqlDatabase db) : QSqlTableModel(parent, db)
{

}

QueryEditModel::~QueryEditModel(){
 if(isDirty())
  submit();
}

QVariant QueryEditModel::data ( const QModelIndex & index, int role ) const
{
 if (!index.isValid())
     return QVariant();

 // let the base class handle all other cases
 return QSqlTableModel::data( index, role );
}

bool QueryEditModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 QVariant hdr;
 if(role == Qt::EditRole)
 {
  hdr = headerData(index.column(), Qt::Horizontal, Qt::DisplayRole);
 }
 return QSqlTableModel::setData( index, value, role );

}

void QueryEditModel::setTabName(QString name)
{
 tabName = name;
}

QVariant QueryEditModel::edit(const QModelIndex &index) const
{
 return QVariant();
}
