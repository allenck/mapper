#include "systemconsole2.h"
#include "qsignalmapper.h"
#include "ui_systemconsole2.h"
#include <QMenu>
#include <QColor>

SystemConsole2::SystemConsole2(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SystemConsole2)
{
 ui->setupUi(this);
    connect(ui->btnClose, &QPushButton::clicked, [=]{
     close();
 });
 _instance = this;
 setScheme(scheme);
 flags = windowFlags();
 connect(ui->chkAlwaysOnTop, &QCheckBox::clicked, [=] (bool checked){
     if(checked)
      setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
     else
      setWindowFlags(windowFlags());
 });
}

/*static*/ SystemConsole2* SystemConsole2::_instance =nullptr;

/*static*/ SystemConsole2* SystemConsole2::instance() {
 // if(_instance == nullptr)
 //  _instance = new SystemConsole2();
 return _instance;
}

SystemConsole2::~SystemConsole2()
{
 delete ui;
}

void SystemConsole2::message(QString txt)
{
 ui->console->append(txt);
}

/**
 * Set the console colour scheme
 *
 * @param which the scheme to use
 */
/*public*/  void SystemConsole2::setScheme(int which)
{
 scheme = which;

 if (schemes.isEmpty())
 {
  defineSchemes();
 }

 SystemConsole2::Scheme s;

 s = schemes.at(which);

 ui->console->setStyleSheet(QString("QTextEdit {color: rgb(%1, %2, %3); background-color: rgb(%4, %5, %6); background-attachment: scroll;}")
                            .arg(s.foreground.red()).arg(s.foreground.green()).arg(s.foreground.blue())
                            .arg(s.background.red()).arg(s.background.green()).arg(s.background.blue()));

 if (schemeGroup != NULL)
 {
//     schemeGroup->setSelected(schemeMenu.getItem(scheme).getModel(), true);
 }
}

/**
 * Method to define console colour schemes
 */
/*private*/ void SystemConsole2::defineSchemes() {
    schemes = QList<SystemConsole2::Scheme>();
    schemes.append( SystemConsole2::Scheme(tr("Green on Black"), Qt::green, Qt::black));
    schemes.append( SystemConsole2::Scheme(tr("Orange on Black"), QColor(255,165,0), Qt::black));
    schemes.append( SystemConsole2::Scheme(tr("White On Black"), Qt::white, Qt::black));
    schemes.append( SystemConsole2::Scheme(tr("Black On White"), Qt::black, Qt::white));
    schemes.append( SystemConsole2::Scheme(tr("White On Blue"), Qt::white, Qt::blue));
    schemes.append( SystemConsole2::Scheme(tr("Black On LightGray"), Qt::black, QColor(211,211,211)));
    schemes.append( SystemConsole2::Scheme(tr("Black On Gray"), Qt::black, Qt::gray));
    schemes.append( SystemConsole2::Scheme(tr("White On Gray"), Qt::white, Qt::gray));
    schemes.append( SystemConsole2::Scheme(tr("White On DarkGray"), Qt::white, QColor(112,128,144)));
    schemes.append( SystemConsole2::Scheme(tr("Green On DarkGray"), Qt::green, QColor(112,128,144)));
    schemes.append( SystemConsole2::Scheme(tr("Orange On DarkGray"), QColor(255,165,0), QColor(112,128,144)));
}
/*public*/  QTextStream* SystemConsole2::getOutputStream() {
    return this->outputStream;
}

/*public*/  QTextStream* SystemConsole2::getErrorStream() {
    return this->errorStream;
}

/**
 * Retrieve the current console colour scheme
 *
 * @return selected colour scheme
 */
/*public*/  int SystemConsole2::getScheme() {
    return scheme;
}

/*public*/  QList<SystemConsole2::Scheme> SystemConsole2::getSchemes()
{
 //return this.schemes.toArray(new Scheme[this.schemes.size()]);
 return schemes;
}

void SystemConsole2::On_contextMenu(QPoint pt)
{
 popup = ui->console->createStandardContextMenu();
 QAction* rbMenuItem;

 schemeMenu = new QMenu(tr("Color scheme"));
 schemeGroup = new QActionGroup(this);
 QSignalMapper* mapper = new QSignalMapper();
 //int i = 0;
 for (int i=0; i < schemes.count(); i++)
 {
  Scheme s = schemes.at(i);
  rbMenuItem = new QAction(s.description, ui->console);
  rbMenuItem->setCheckable(true);

//  rbMenuItem.addActionListener((ActionEvent event) -> {
//      setScheme(schemes.indexOf(s));
//  });
  mapper->setMapping(rbMenuItem, i++);
  connect(rbMenuItem, SIGNAL(toggled(bool)), mapper, SLOT(map()));
  rbMenuItem->setChecked(getScheme() == /*schemes.indexOf(s)*/ i);
  schemeMenu->addAction(rbMenuItem);
  schemeGroup->addAction(rbMenuItem);
 }
 connect(mapper, SIGNAL(mapped(int)), this, SLOT(setScheme(int)));
 popup->addMenu(schemeMenu);

 // Define the wrap style sub-menu
 wrapMenu = new QMenu(tr("Wrap style"));
 wrapGroup = new QActionGroup(this);
 rbMenuItem = new QAction(tr("No text wrapping"), ui->console);
// rbMenuItem.addActionListener((ActionEvent event) -> {
//     setWrapStyle(WRAP_STYLE_NONE);
// });
 connect(rbMenuItem, SIGNAL(toggled(bool)), this, SLOT(On_setWrapStyleNone()));
 rbMenuItem->setCheckable(true);
 rbMenuItem->setChecked(getWrapStyle() == WRAP_STYLE_NONE);
 wrapMenu->addAction(rbMenuItem);
 wrapGroup->addAction(rbMenuItem);

 rbMenuItem = new QAction(tr("Wrap text at end of line"), ui->console);
// rbMenuItem.addActionListener((ActionEvent event) -> {
//     setWrapStyle(WRAP_STYLE_LINE);
// });
 connect(rbMenuItem, SIGNAL(triggered(bool)), this, SLOT(On_wrapLine()));
 rbMenuItem->setCheckable(true);
 rbMenuItem->setChecked(getWrapStyle() == WRAP_STYLE_LINE);
 wrapMenu->addAction(rbMenuItem);
 wrapGroup->addAction(rbMenuItem);

 rbMenuItem = new QAction(tr("ConsoleWrapStyleWord"), ui->console);
// rbMenuItem.addActionListener((ActionEvent event) -> {
//     setWrapStyle(WRAP_STYLE_WORD);
// });
 connect(rbMenuItem, SIGNAL(triggered()), this, SLOT(On_wrapWord()));
 rbMenuItem->setCheckable(true);
 rbMenuItem->setChecked(getWrapStyle() == WRAP_STYLE_WORD);
 wrapMenu->addAction(rbMenuItem);
 wrapGroup->addAction(rbMenuItem);

 popup->addMenu(wrapMenu);
 QAction* copy = new QAction(tr("Copy"), this);
 popup->addSeparator();
 connect(copy, SIGNAL(triggered()), this, SLOT(On_copy_clicked()));

// // Bind pop-up to objects
// MouseListener popupListener = new PopupListener();
// console->addMouseListener(popupListener);
// frame.addMouseListener(popupListener);
 popup->exec(QCursor::pos());
}

/**
 * Retrieve the current console wrapping style
 *
 * @return current wrapping style - one of
 * <ul>
 * <li>{@link #WRAP_STYLE_NONE} No wrapping
 * <li>{@link #WRAP_STYLE_LINE} Wrap at end of line
 * <li>{@link #WRAP_STYLE_WORD} Wrap by word boundaries (default)
 * </ul>
 */
/*public*/  int SystemConsole2::getWrapStyle() {
    return wrapStyle;
}

/**
 * Set the console font size
 *
 * @param size point size of font between 6 and 24 point
 */
/*public*/  void SystemConsole2::setFontSize(int size)
{
 updateFont(fontFamily, fontStyle, (fontSize = size < 6 ? 6 : size > 24 ? 24 : size));
}

/**
 * Retrieve the current console font size (default 12 point)
 *
 * @return selected font size in points
 */
/*public*/  int SystemConsole2::getFontSize() {
    return fontSize;
}

/**
 * Set the console font style
 *
 * @param style one of
 *              {@link Font#BOLD}, {@link Font#ITALIC}, {@link Font#PLAIN}
 *              (default)
 */
/*public*/  void SystemConsole2::setFontStyle(int style)
{
 if (style == BOLD || style == ITALIC || style == PLAIN || style == (BOLD | ITALIC))
 {
  fontStyle = style;
 }
 else
 {
  fontStyle = PLAIN;
 }
 updateFont(fontFamily, fontStyle, fontSize);
}

/*public*/  void SystemConsole2::setFontFamily(QString family)
{
 updateFont((fontFamily = family), fontStyle, fontSize);
}

/*public*/  QString SystemConsole2::getFontFamily() {
    return fontFamily;
}

/**
 * Retrieve the current console font style
 *
 * @return selected font style - one of
 *         {@link Font#BOLD}, {@link Font#ITALIC}, {@link Font#PLAIN}
 *         (default)
 */
/*public*/  int SystemConsole2::getFontStyle() {
    return fontStyle;
}

/**
 * Update the system console font with the specified parameters
 *
 * @param style font style
 * @param size  font size
 */
/*private*/ void SystemConsole2::updateFont(QString family, int style, int size)
{
 ui->console->setFont( QFont(family, size, style&BOLD?QFont::Bold:QFont::Normal, style& ITALIC?QFont::StyleItalic:QFont::StyleNormal));
 QFont f = ui->console->font();
 //log->debug(QString("font family = %1, size = %2, weight = %3, style = %4").arg(f.family()).arg(f.pointSize()).arg(f.weight()).arg(f.style()));
}

SystemConsole2::Scheme::Scheme(QString description, QColor foreground, QColor background)
{
 this->foreground = foreground;
 this->background = background;
 this->description = description;
}
bool SystemConsole2::Scheme::equals(Scheme other)
{
 if(this->description == other.description)
  return true;
 return false;
}
