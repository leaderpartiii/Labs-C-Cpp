#ifndef TOOLBAR_H
#define TOOLBAR_H
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

class Toolbar : public QWidget
{
	Q_OBJECT;

  public:
	explicit Toolbar(QWidget *parent = nullptr);

	int getHeight();
	int getWidth();
	int getMines();
	void change();
	~Toolbar();
  signals:
	void exchange(int h, int w, int m);

  private:
	QLineEdit *widthEdit_;
	QLineEdit *heightEdit_;
	QLineEdit *minesEdit_;
	QPushButton *applyButton_;
	QHBoxLayout *toolBarLayout_;
};

#endif	  // TOOLBAR_H
