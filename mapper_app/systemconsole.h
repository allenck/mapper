#ifndef SYSTEMCONSOLE_H
#define SYSTEMCONSOLE_H
#include "jtextarea.h"
#include "console_global.h"

class Logger;
//class JFrame;
class QCheckBox;
//class UserPreferencesManager;
class QActionGroup;
class QMainWindow;
class QTextStream;
class  SystemConsole : public QObject
{
 Q_OBJECT
public:
 explicit SystemConsole(QObject *parent = 0);
    ~SystemConsole();
 /*public*/  static void create();
 /*public*/  void setFontSize(int size);
 /*public*/  void setWrapStyle(int style);
 /*public*/  int getWrapStyle();
 /*public*/  int getFontSize();
 /*public*/  void setFontStyle(int style);
 /*public*/  void setFontFamily(QString family);
 /*public*/  QString getFontFamily();
 /*public*/  int getFontStyle();
 /*public*/  QTextStream* getOutputStream();
 /*public*/  QTextStream* getErrorStream();
 /*public*/  int getScheme();
 /*public*/ static /*final*/ int STD_ERR;// = 1;
 /*public*/ static /*final*/ int STD_OUT;// = 2;
 /*public*/ void closeConsole();

 /*public*/  /*static*/ /*final*/ class Scheme
 {
 public:
  /*public*/  QColor foreground;
  /*public*/  QColor background;
  /*public*/  QString description;
   Scheme(QString description, QColor foreground, QColor background);
 };

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

 /*public*/  static SystemConsole* getInstance();
 /*public*/  static QMainWindow* getFrame(QWidget* parent);
 /*public*/  QMainWindow* openFrame(QWidget* parent);
 /*public*/ QList<Scheme*>* getSchemes();
/*private*/ void updateTextArea(/*final*/ QString text, /*final*/ int which);

#if 0

 /**
  * Class to deal with handling popup menu
  */
 /*public*/  /*final*/ class PopupListener extends MouseAdapter {

  //@Override
  /*public*/  void mousePressed(MouseEvent e) {
      maybeShowPopup(e);
  }

  //@Override
  /*public*/  void mouseReleased(MouseEvent e) {
 };
#endif
signals:

public slots:
  void On_copy_clicked();
  void On_close_clicked();
  void On_autoScroll_toggled(bool );
  void On_alwaysOnTop_toggled( bool);
  void On_contextMenu(QPoint);
  void On_wrapLine();
  void On_wrapWord();
  void On_appendText(QString);
  /*public*/  void setScheme(int which);
  void On_setWrapStyleNone();

private:
// /*private*/ void maybeShowPopup(QMouseEvent* e);
 /*private*/ /*final*/ JTextArea* console;

 /*private*/ /*final*/ QTextStream* originalOut;
 /*private*/ /*final*/ QTextStream* originalErr;

 /*private*/ /*final*/ QTextStream* outputStream;
 /*private*/ /*final*/ QTextStream* errorStream;

 /*private*/ QMainWindow* frame;// = null;

 /*private*/ /*final*/ QMenu*popup;// = new JPopupMenu();

 /*private*/ QAction* copySelection;// = null;

 /*private*/ QMenu* wrapMenu;// = null;
 /*private*/ QActionGroup* wrapGroup;// = null;

 /*private*/ QMenu* schemeMenu;// = null;
 /*private*/ QActionGroup* schemeGroup;// = null;

 /*private*/ QList<Scheme*>* schemes;

 /*private*/ int scheme;// = 0; // Green on Black

 /*private*/ int fontSize;// = 12;

 /*private*/ int fontStyle ;//= Font.PLAIN;

 /*private*/ QString fontFamily;// = "Monospaced";  //NOI18N
 /*private*/ int wrapStyle;// = WRAP_STYLE_WORD;

 /*private*/ static SystemConsole* _instance;

 ///*private*/ UserPreferencesManager* pref;

 /*private*/ QCheckBox* autoScroll;
 /*private*/ QCheckBox* alwaysOnTop;

 /*private*/ /*final*/ QString alwaysScrollCheck;// = this.getClass().getName() + ".alwaysScroll"; //NOI18N
 /*private*/ /*final*/ QString alwaysOnTopCheck;// = this.getClass().getName() + ".alwaysOnTop";   //NOI18N
 /*private*/ void createFrame(QWidget* parent);
 //
 /*private*/ void doAutoScroll(/*final*/ JTextArea* ta, /*final*/ bool scroll);
 /*private*/ QDataStream* outStream(/*final*/ int which);
 /*private*/ void redirectSystemStreams();
  /*private*/ void updateFont(QString family, int style, int size);
  /*private*/ void defineSchemes() ;
//  /*private*/ QMap<Thread, StackTraceElement[]> traces;
  /*private*/ void performStackTrace() ;
 //Logger* log;
  void setAlwaysOnTop(bool b);

};

#endif // SYSTEMCONSOLE_H
