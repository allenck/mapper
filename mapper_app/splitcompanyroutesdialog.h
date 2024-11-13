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
  void setOrginalCompany(int companyKey);

 public slots:
  void fillCompanies();

 private slots:
  void btnApply_clicked();

 private:
  Ui::SplitCompanyRoutesDialog *ui;
  QList<CompanyData*> _companyList;
  SQL* sql;
  CompanyData* cd1;
  CompanyData* cd2;
  bool checkParameters();
  QList<SegmentData*> segmentList;
};

#endif // SPLITCOMPANYROUTESDIALOG_H
