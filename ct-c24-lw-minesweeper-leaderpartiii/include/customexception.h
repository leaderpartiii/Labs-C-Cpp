#ifndef CUSTOMEXCEPTION_H
#define CUSTOMEXCEPTION_H
#include <QException>
#include <QMessageBox>

class CustomException : public QException
{
  public:
	QString message_;
	CustomException(const QString &message);
	void setMessage() { QMessageBox::critical(nullptr, "Error", message_.toUtf8().data()); }
};

#endif	  // CUSTOMEXCEPTION_H
