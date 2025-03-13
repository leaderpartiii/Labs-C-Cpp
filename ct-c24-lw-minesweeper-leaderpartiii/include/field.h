#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "custombutton.h"
#include "gameover.h"
#include "toolbar.h"

#include <QCoreApplication>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QRadioButton>
#include <QSettings>
#include <QTime>

class Field : public QMainWindow
{
	Q_OBJECT

  public:
	Field(int col, int row, int cntMines, bool ghost, QSettings *settings, QMainWindow *parent = 0);

	void buildButtons();

	void buildField();

	bool isNormal(QPair< int, int > coords);

	bool isMine(QPair< int, int > temp);
	bool isNums(QPair< int, int > temp);

	void randomFillAreaMines(int row, int col);

	void calculateSurroundings(QPair< int, int > coords);

	void walking(QPair< int, int > coords);

	QSet< QPair< int, int > > randomGeneratorSet(int row, int col);

	void setImage(CustomButton *button, QString filename);

	void restart();
	void setFieldParams(int height, int wight, int mins);
	bool isAllPressed();
	bool isOkey(int row, int col, int cntMine);
	void setGhostMode();
	void highLightMines(int row, int col);
	void saveState();
	void isFinal();

	void setState();
	void clearGrid();

	~Field();
  signals:
	void leftButtonClicked(int x, int y);
	void rightButtonClicked(int x, int y);
	void win(QString fileName, QString message);
	void lose(QString fileName, QString message);

  private:
	QGridLayout *grid_;
	int mines_ = 0;
	bool ghostMode_;
	QPushButton *save_;
	QPushButton *restartButton_;
	QPushButton *setSave_;
	QWidget *centralWidget_;
	int columns_ = 10;
	int rows_ = 10;
    int sizeButt_ = 25;
	int cntMines_ = 10;
	GameOver *final_;
	Toolbar *toolbar_;
	QRadioButton *radioButton_;
	QSettings *settings_;
	QWidget *gridWidget;
	QVBoxLayout *topLayout;
	QHBoxLayout *leastLaytout;
	QString path_ = ":/images/images";
	QMap< QString, QString > files{
		{ "fileNameFlag", path_ + "/flag.png" },	   { "fileNameBomb", path_ + "/bomb.png" },
		{ "fileNameExpBomb", path_ + "/expbomb.png" }, { "fileNamePress", path_ + "/press.png" },
		{ "fileNameUnpress", path_ + "/unpress.jpg" }, { "fileNameSad", path_ + "/sad.jpg" },
		{ "fileNameFunny", path_ + "/funny.png" },	   { "fileTemplateNums", path_ + "/nums/num" },
		{ "fileNameQuestion", path_ + "/inform.png" }
	};

  private slots:
	void handleRightButtonClicked(int x, int y);
	void handleMidButtonClicked(int x, int y);
	void handleLeftButtonClicked(int x, int y);
};
#endif	  // MAINWINDOW_H
