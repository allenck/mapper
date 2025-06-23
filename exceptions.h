#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <QObject>
#include <QString>
#include <QException>

class /*JAVAQTSHARED_EXPORT*/ Throwable : public QException
{
 //Q_OBJECT
public:
 void raise() const { throw *this; }
   Throwable *clone() const { return new Throwable(*this); }

    Throwable(QString msg, QString localMsg, Throwable* cause)
    {
     this->msg = msg;
     this->localMsg = localMsg;
     this->cause = cause;
     this->name = "Throwable";
    }

    Throwable(QString msg, QString localMsg)
    {
     this->msg = msg;
     this->localMsg = localMsg;
     this->cause = NULL;
     this->name = "Throwable";
    }

    Throwable(QString msg, Throwable* cause)
    {
     this->msg = msg;
     this->localMsg = "";
     this->cause = cause;
    this->name = "Throwable";
    }
    Throwable(QString msg)
    {
     this->msg = msg;
     this->localMsg = "";
     this->cause = cause;
    this->name = "Throwable";
    }


    Throwable(Throwable* cause)
    {
     this->cause = cause;
     this->msg = cause->msg;
     this->localMsg = cause->getLocalizedMessage();
    this->name = "Throwable";
    }
    Throwable()
    {
     this->cause = NULL;
     this->msg = "";
     this->localMsg = "";
     this->name = "Throwable";
    }
    ~Throwable() throw(){}
    QString msg;
    QString localMsg;
    QString name;
    virtual QString getMessage() {return msg;}
    virtual QString getLocalizedMessage() {return localMsg;}
    virtual QString toString() {
     QString str = QString("exception %1: %2").arg(this->name).arg(msg);
     return str;
    }
    Throwable* cause;
    Throwable* getCause() {return cause;}
};

class Exception : public Throwable
{
// Q_OBJECT
// Q_INTERFACES(Throwable)
public:
 Exception() : Throwable() {name = "Exception";}
 Exception(QString msg) : Throwable(msg) {name = "Exception";}
 Exception(QString msg, Throwable* throwable) : Throwable(msg, throwable) {name = "Exception";}
 Exception(QString msg, QString localMsg) : Throwable(msg, localMsg) {name = "Exception";}
 Exception(QString msg, QString localMsg, Exception* exception) : Throwable(msg, localMsg, exception)
 {name = "Exception";}
 Exception(Exception* exception) : Throwable(exception) {name = "Exception";}
 Exception(Throwable* exception) : Throwable(exception) {name = "Exception";}
};

class DupSegmentException : public Exception
{
 public:
  DupSegmentException(QString msg) : Exception(msg) {name = "DupSegment";}
};

class IllegalArgumentException : public Exception
{
 public:
  IllegalArgumentException(QString msg) : Exception(msg) {name = "IllegalArgument";}
};
class RecordNotFoundException : public Exception
{
 public:
  RecordNotFoundException(QString msg) : Exception(msg) {name = "RecordNotFoundException";}
};
class FileNotFoundException : public Exception
{
 public:
  FileNotFoundException(QString msg) : Exception(msg) {name = "FileNotFoundException";}
};
class NullPointerException : public Exception
{
 public:
  NullPointerException(QString msg) : Exception(msg) {name = "NullPointerException";}
};
class ApplicationException : public Exception
{
 public:
  ApplicationException(QString msg) : Exception(msg) {name = "NullPointerException";}
};

#endif // EXCEPTIONS_H
