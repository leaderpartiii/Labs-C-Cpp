#ifndef DIALOGWINDOW_H
#define DIALOGWINDOW_H
#include <QDialog>
#include <QLineEdit>

class Dialog : public QDialog
{
	Q_OBJECT

  public:
	Dialog(QWidget *parent = nullptr);
	int width() const;
	int length() const;
	int mines() const;

  private:
	QLineEdit *widthEdit_;
	QLineEdit *lengthEdit_;
	QLineEdit *minesEdit_;
};
#endif	  // DIALOGWINDOW_H
