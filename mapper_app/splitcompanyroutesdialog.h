#ifndef SPLITCOMPANYROUTESDIALOG_H
#define SPLITCOMPANYROUTESDIALOG_H

#include <QDialog>
#include "data.h"
#include "sql.h"

namespace Ui {
 class SplitCompanyRoutesDialog;
}

class SplitCompanyRoutesDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit SplitCompanyRoutesDialog(QWidget *parent = nullptr);
  ~SplitCompanyRoutesDialog();

 public slots:
  void fillCompanies();

 private slots:
  void btnApply_clicked();

 private:
  Ui::SplitCompanyRoutesDialog *ui;
  QList<CompanyData*> _companyList;
  SQL* sql;
};

#endif // SPLITCOMPANYROUTESDIALOG_H
