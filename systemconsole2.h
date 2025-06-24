#ifndef SYSTEMCONSOLE2_H
#define SYSTEMCONSOLE2_H

#include <QDialog>
#include <QActionGroup>

namespace Ui {
 class SystemConsole2;
}

class QTextStream;
class SystemConsole2 : public QDialog
{
  Q_OBJECT
  public:
  explicit SystemConsole2(QWidget *parent = nullptr);
  /*public*/  void setScheme(int which);
  /*public*/  /*static*/ /*final*/ class Scheme
  {
  public:
   /*public*/  QColor foreground;
   /*public*/  QColor background;
   /*public*/  QString description;
    Scheme() {}
    Scheme(QString description, QColor foreground, QColor background);
    bool equals(Scheme other);
  };
  /*public*/  int getWrapStyle();
  /*public*/  void setFontSize(int size);
  /*public*/  int getFontSize();
  /*public*/  void setFontStyle(int style);
  /*public*/  void setFontFamily(QString family);
  /*public*/  QString getFontFamily();
  /*public*/  int getFontStyle();
  /*private*/ void updateFont(QString family, int style, int size);

  enum WRAP
  {
   WRAP_STYLE_NONE = 0x00,
   WRAP_STYLE_LINE = 0x01,
   WRAP_STYLE_WORD = 0x02
  };
  enum FONTSTYLE
  {
   PLAIN = 0,
   BOLD = 1,
   ITALIC = 2
  };
 public:
  ~SystemConsole2();
  static SystemConsole2* instance();
  /*public*/ QList<Scheme> getSchemes();
  /*public*/  QTextStream* getOutputStream();
  /*public*/  QTextStream* getErrorStream();
  /*public*/  int getScheme();

 public slots:
  void message(QString);
  void On_contextMenu(QPoint pt);

 signals:
  void outMessage(QString);

 private:
  Ui::SystemConsole2 *ui;
  static SystemConsole2* _instance;
  /*private*/ void defineSchemes();
  /*private*/ QList<SystemConsole2::Scheme> schemes;
  /*private*/ int scheme = 0; // Green on Black
  /*private*/ int fontSize = 12;
  /*private*/ int fontStyle= QFont::StyleNormal;
  /*private*/ QString fontFamily = "Monospaced";  //NOI18N
  /*private*/ int wrapStyle = WRAP_STYLE_WORD;
  int flags;
  /*private*/ /*final*/ QTextStream* originalOut = nullptr;
  /*private*/ /*final*/ QTextStream* originalErr = nullptr;

  /*private*/ /*final*/ QTextStream* outputStream = nullptr;
  /*private*/ /*final*/ QTextStream* errorStream = nullptr;
  /*private*/ QActionGroup* schemeGroup = nullptr;
  QMenu* popup;
  QMenu* schemeMenu;
  QMenu* wrapMenu;
  QActionGroup* wrapGroup;
};

#endif // SYSTEMCONSOLE2_H
