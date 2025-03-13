#include "include/field.h"

#include "include/custombutton.h"
#include "include/customexception.h"
#include "include/gameover.h"
#include "include/toolbar.h"

#include <QApplication>
#include <QDebug>
#include <QException>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPair>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QVariant>

static bool wasUsed = false;

QVector< QPair< int, int > > dxdy{ { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };

#define rnd(n) QRandomGenerator::global()->bounded(n)
bool Field::isOkey(int row, int col, int cntMine)
{
	if (col <= 0 || row <= 0 || cntMine <= 0)
	{
		CustomException("Columns, rows and count of mines must be positive integers").setMessage();
		return false;
	}
	if (cntMine >= row * col)
	{
		CustomException("Count of mines must be less than multiply row and col").setMessage();
		return false;
	}
	return true;
}

Field::Field(int col, int row, int cntMine, bool ghost, QSettings *settings, QMainWindow *parent) :
	QMainWindow(parent), ghostMode_(ghost), columns_(col), rows_(row), cntMines_(cntMine), settings_(settings)
{
	if (!isOkey(row, col, cntMine))
	{
		rows_ = 10;
		columns_ = 10;
		cntMines_ = 10;
	}

	save_ = new QPushButton("Save");
	restartButton_ = new QPushButton("Restart");
	setSave_ = new QPushButton("Set saved game");
	grid_ = new QGridLayout();
	toolbar_ = new Toolbar();

	topLayout = new QVBoxLayout();
	topLayout->addWidget(toolbar_);
	if (ghostMode_)
	{
		radioButton_ = new QRadioButton("Hints:", this);
		topLayout->addWidget(radioButton_);
		connect(radioButton_, &QRadioButton::clicked, this, &Field::setGhostMode);
	}

	leastLaytout = new QHBoxLayout();

	leastLaytout->addWidget(save_);
	leastLaytout->addWidget(restartButton_);
	leastLaytout->addWidget(setSave_);

	topLayout->addLayout(leastLaytout);

	QWidget *gridWidget = new QWidget(this);

	gridWidget->setLayout(grid_);

	centralWidget_ = new QWidget(this);
	setCentralWidget(centralWidget_);

	QVBoxLayout *mainLayout = new QVBoxLayout();

	centralWidget_->setLayout(mainLayout);
	mainLayout->addLayout(topLayout);
	mainLayout->addWidget(gridWidget);

	buildField();

	final_ = new GameOver();

	connect(this, &Field::win, final_, &GameOver::templateFinal);
	connect(this, &Field::lose, final_, &GameOver::templateFinal);

	connect(toolbar_, &Toolbar::exchange, this, &Field::setFieldParams);

	connect(restartButton_, &QPushButton::clicked, this, &Field::restart);

	connect(setSave_, &QPushButton::clicked, this, &Field::setState);
	connect(save_, &QPushButton::clicked, this, &Field::saveState);
}

void Field::saveState()
{
	settings_->beginGroup("GameState");

	settings_->setValue("Rows", rows_);
	settings_->setValue("Columns", columns_);
	settings_->setValue("MineCount", cntMines_);
	settings_->setValue("Mines", mines_);
	settings_->setValue("WasUsed", wasUsed);
	settings_->setValue("GhostMode", ghostMode_);

	settings_->endGroup();

	settings_->beginGroup("Grid");

	for (int i = 0; i < rows_; i++)
	{
		for (int j = 0; j < columns_; j++)
		{
			CustomButton *button = static_cast< CustomButton * >(grid_->itemAtPosition(i, j)->widget());
			QString key = QString("Grid_%1_%2").arg(i).arg(j);

			settings_->setValue(key + "_Value", button->getCntTypes());
			settings_->setValue(key + "_Type", button->getType());
			settings_->setValue(key + "_Press", button->getPress());
		}
	}
	settings_->endGroup();
}

void Field::setState()
{
	if (!settings_->contains("GameState/Rows") || !settings_->contains("GameState/Columns") || !settings_->contains("GameState/MineCount"))
	{
		qDebug() << "Invalid save game file";
		return;
	}
	clearGrid();

	settings_->beginGroup("GameState");

	rows_ = settings_->value("Rows").toInt();
	columns_ = settings_->value("Columns").toInt();
	cntMines_ = settings_->value("MineCount").toInt();
	mines_ = settings_->value("Mines").toInt();
	if (!isOkey(rows_, columns_, cntMines_))
		return;

	grid_->setSpacing(0);
	grid_->setRowStretch(rows_, 1);
	grid_->setColumnStretch(columns_, 1);

	wasUsed = settings_->value("WasUsed").toBool();
	ghostMode_ = settings_->value("GhostMode").toBool();

	settings_->endGroup();

	settings_->beginGroup("Grid");

	for (int i = 0; i < rows_; i++)
	{
		for (int j = 0; j < columns_; j++)
		{
			QString key = QString("Grid_%1_%2").arg(i).arg(j);
			CustomButton *button = new CustomButton(i, j, CustomButton::Types::empty, this);
			button->setFixedSize(sizeButt_, sizeButt_);

			switch (settings_->value(key + "_Type").toInt())
			{
			case (CustomButton::Types::mines):
				button->setType(CustomButton::Types::mines);
				button->setCntTypes(-1);
				if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::flagged)
				{
					setImage(button, files["fileNameFlag"]);
					button->setPress(CustomButton::Pressing::flagged);
				}
				else if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::unpressed)
				{
					button->setPress(CustomButton::Pressing::unpressed);

					setImage(button, files["fileNameUnpress"]);
				}
				else if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::pressed)
				{
					button->setEnabled(true);
					button->setPress(CustomButton::Pressing::pressed);
					setImage(button, files["fileNamePress"]);
				}
				break;
			case (CustomButton::Types::nums):
				button->setType(CustomButton::Types::nums);
				button->setCntTypes(settings_->value(key + "_Value").toInt());
				if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::pressed)
				{
					setImage(button, files["fileTemplateNums"] + QString::number(button->getCntTypes()) + ".png");
					button->setPress(CustomButton::Pressing::pressed);
					button->setEnabled(true);
				}
				else if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::flagged)
				{
					setImage(button, files["fileNameFlag"]);
					button->setPress(CustomButton::Pressing::flagged);
					button->setEnabled(true);
				}
				else if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::unpressed)
				{
					setImage(button, files["fileNameUnpress"]);
					button->setPress(CustomButton::Pressing::unpressed);
				}
				break;
			case (CustomButton::Types::empty):
				button->setType(CustomButton::Types::empty);
				button->setCntTypes(0);
				if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::pressed)
				{
					setImage(button, files["fileNamePress"]);
					button->setEnabled(true);
					button->setPress(CustomButton::Pressing::pressed);
				}
				else if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::unpressed)
				{
					setImage(button, files["fileNameUnpress"]);
					button->setPress(CustomButton::Pressing::unpressed);
				}
				else if (settings_->value(key + "_Press").toInt() == CustomButton::Pressing::flagged)
				{
					setImage(button, files["fileNameFlag"]);
					button->setPress(CustomButton::Pressing::flagged);
				}
			}

			button->setFixedSize(sizeButt_, sizeButt_);
			grid_->addWidget(button, i, j);

			connect(button, &QPushButton::clicked, button, &CustomButton::handleButtonClicked);

			connect(button, &CustomButton::leftButtonClicked, this, &Field::handleLeftButtonClicked);

			connect(button, &CustomButton::rightButtonClicked, this, &Field::handleRightButtonClicked);

			connect(button, &CustomButton::midButtonClicked, this, &Field::handleMidButtonClicked);
		}
	}
	settings_->endGroup();
}

void setEmptyIcon(CustomButton *button)
{
	button->setIcon(QIcon());
	button->setIconSize(button->size());
}

void Field::setGhostMode()
{
	if (radioButton_->isChecked())
	{
		for (int i = 0; i < rows_; ++i)
		{
			for (int j = 0; j < columns_; ++j)
			{
				CustomButton *button = static_cast< CustomButton * >(grid_->itemAtPosition(i, j)->widget());
				if (isMine(qMakePair(i, j)) && button->getPress() != CustomButton::Pressing::flagged)
				{
					setEmptyIcon(button);
					setImage(button, files["fileNameBomb"]);
				}
				else if (isNums(qMakePair(i, j)) && button->getPress() != CustomButton::Pressing::pressed)
				{
					setEmptyIcon(button);
					setImage(button, files["fileTemplateNums"] + QString::number(button->getCntTypes()) + ".png");
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < rows_; ++i)
		{
			for (int j = 0; j < columns_; ++j)
			{
				CustomButton *button = static_cast< CustomButton * >(grid_->itemAtPosition(i, j)->widget());
				if (isMine(qMakePair(i, j)) && button->getPress() != CustomButton::Pressing::flagged)
				{
					setEmptyIcon(button);
					setImage(button, files["fileNameUnpress"]);
				}
				else if (isNums(qMakePair(i, j)) && button->getPress() != CustomButton::Pressing::pressed)
				{
					button->setStyleSheet("");
					setEmptyIcon(button);
					setImage(button, files["fileNameUnpress"]);
				}
			}
		}
	}
}
void Field::clearGrid()
{
	while (grid_->count() > 0)
	{
		QLayoutItem *item = grid_->takeAt(0);
		if (item->widget())
		{
			item->widget()->blockSignals(true);
			item->widget()->deleteLater();
		}
	}
}

void Field::restart()
{
	mines_ = 0;
	wasUsed = false;
	clearGrid();
	buildField();
}

void Field::setFieldParams(int height, int width, int mins)
{
	if (!isOkey(width, height, mins))
		return;

	columns_ = height;
	rows_ = width;
	cntMines_ = mins;
	restart();
}

void Field::setImage(CustomButton *button, QString filename)
{
	QPixmap pixmap;
	if (pixmap.load(filename))
	{
		setEmptyIcon(button);
		button->setIcon(pixmap);
		button->setIconSize(button->size());
	}
	else
	{
		qDebug() << "Error loading image: " << filename;
	}
}

void Field::buildField()
{
	grid_->setSpacing(0);
	grid_->setRowStretch(rows_, 1);
	grid_->setColumnStretch(columns_, 1);

	for (int i = 0; i < rows_; ++i)
	{
		for (int j = 0; j < columns_; ++j)
		{
			CustomButton *button = new CustomButton(i, j, CustomButton::Types::empty, this);
			button->setFixedSize(sizeButt_, sizeButt_);
			setImage(button, files["fileNameUnpress"]);
			grid_->addWidget(button, i, j);

			connect(button, &QPushButton::clicked, button, &CustomButton::handleButtonClicked);
			connect(button, &CustomButton::leftButtonClicked, this, &Field::handleLeftButtonClicked);
			connect(button, &CustomButton::rightButtonClicked, this, &Field::handleRightButtonClicked);
			connect(button, &CustomButton::midButtonClicked, this, &Field::handleMidButtonClicked);

		}
	}
}

bool Field::isMine(QPair< int, int > temp)
{
	return static_cast< CustomButton * >(grid_->itemAtPosition(temp.first, temp.second)->widget())->getType() == CustomButton::Types::mines &&
		   static_cast< CustomButton * >(grid_->itemAtPosition(temp.first, temp.second)->widget())->getCntTypes() == -1;
}

bool Field::isNums(QPair< int, int > temp)
{
	return static_cast< CustomButton * >(grid_->itemAtPosition(temp.first, temp.second)->widget())->getType() == CustomButton::Types::nums &&
		   static_cast< CustomButton * >(grid_->itemAtPosition(temp.first, temp.second)->widget())->getCntTypes() > 0;
}

bool Field::isNormal(QPair< int, int > temp)
{
	if (temp.first >= 0 && temp.second >= 0 && temp.first < rows_ && temp.second < columns_)
		return true;
	return false;
}

QSet< QPair< int, int > > Field::randomGeneratorSet(int row, int col)
{
	QSet< QPair< int, int > > temp;

	while (temp.size() < cntMines_)
	{
		int rndCol = rnd(columns_);
		int rndRow = rnd(rows_);
		if (row != rndRow || col != rndCol)
		{
			temp.insert(qMakePair(rndRow, rndCol));
		}
	}
	return temp;
}

void Field::calculateSurroundings(QPair< int, int > coords)
{
	for (QPair< int, int > val : dxdy)
	{
		QPair< int, int > temp = qMakePair(val.first + coords.first, val.second + coords.second);

		if (isNormal(temp) && !isMine(temp))
		{
			int cnt = 0;
			for (QPair< int, int > t : dxdy)
			{
				QPair< int, int > k = qMakePair(t.first + temp.first, t.second + temp.second);
				if (isNormal(k) && isMine(k))
				{
					cnt++;
				}
			}

			CustomButton *widget = static_cast< CustomButton * >(grid_->itemAtPosition(temp.first, temp.second)->widget());

			widget->setType(CustomButton::Types::nums);
			widget->setPress(CustomButton::Pressing::unpressed);
			widget->setCntTypes(cnt);
		}
	}
}

void Field::randomFillAreaMines(int row, int col)
{
	QSet< QPair< int, int > > temp = randomGeneratorSet(row, col);

	for (auto var : temp)
	{
		CustomButton *widget = static_cast< CustomButton * >(grid_->itemAtPosition(var.first, var.second)->widget());

		widget->setType(CustomButton::Types::mines);
		widget->setCntTypes(-1);
		calculateSurroundings(var);
	}
}

void Field::walking(QPair< int, int > coords)
{
	if (!isNormal(coords))
		return;
	CustomButton *widget = static_cast< CustomButton * >(grid_->itemAtPosition(coords.first, coords.second)->widget());
	if (isNums(coords))
	{
		widget->setPress(CustomButton::Pressing::pressed);
		setEmptyIcon(widget);
		setImage(widget, files["fileTemplateNums"] + QString::number(widget->getCntTypes()) + ".png");
		widget->setEnabled(true);
	}
	else if (widget->getPress() == CustomButton::Pressing::unpressed && widget->getCntTypes() == 0)
	{
		widget->setType(CustomButton::Types::empty);
		widget->setPress(CustomButton::Pressing::pressed);
		widget->setCntTypes(-2);

		setImage(widget, files["fileNamePress"]);

		widget->setEnabled(true);

		for (auto t : dxdy)
			walking(qMakePair(coords.first + t.first, coords.second + t.second));
	}
}
bool Field::isAllPressed()
{
	bool flg = true;
	for (int i = 0; i < rows_; ++i)
	{
		for (int j = 0; j < columns_; ++j)
		{
			if (static_cast< CustomButton * >(grid_->itemAtPosition(i, j)->widget())->getPress() == CustomButton::Pressing::unpressed &&
				static_cast< CustomButton * >(grid_->itemAtPosition(i, j)->widget())->getType() != CustomButton::Types::mines)
			{
				flg = false;
				break;
			}
		}
	}
	return flg;
}

void Field::highLightMines(int row, int col)
{
	for (int i = 0; i < rows_; ++i)
	{
		for (int j = 0; j < columns_; ++j)
		{
			CustomButton *but = static_cast< CustomButton * >(grid_->itemAtPosition(i, j)->widget());
			but->setAttribute(Qt::WA_TransparentForMouseEvents, true);
			if ((row != i || col != j) && isMine(qMakePair(i, j)))
			{
				setEmptyIcon(but);
				setImage(but, files["fileNameBomb"]);
			}
		}
	}
}
void Field::isFinal()
{
	if (isAllPressed())
	{
		highLightMines(rows_ + 1, columns_ + 1);
		emit win(files["fileNameFunny"], "You a winning!!");
		final_->exec();
	}
}
void Field::handleLeftButtonClicked(int row, int col)
{
	if (!wasUsed)
	{
		wasUsed = true;

		randomFillAreaMines(row, col);
		walking(qMakePair(row, col));
	}
	else
	{
		CustomButton *widget = static_cast< CustomButton * >(grid_->itemAtPosition(row, col)->widget());

		if (isMine(qMakePair(row, col)))
		{
			setImage(widget, files["fileNameExpBomb"]);
			highLightMines(row, col);
			emit lose(files["fileNameSad"], "You a losing((");
			final_->exec();
		}
		else
		{
			walking(qMakePair(row, col));
		}
	}
	isFinal();
}

void Field::handleRightButtonClicked(int row, int col)
{
	CustomButton *widget = static_cast< CustomButton * >(grid_->itemAtPosition(row, col)->widget());
	if (!wasUsed)
	{
		randomFillAreaMines(row, col);
		wasUsed = true;
	}

	if (widget->getPress() == CustomButton::Pressing::flagged)
	{
		setEmptyIcon(widget);
		setImage(widget, files["fileNameUnpress"]);
		widget->setPress(CustomButton::Pressing::unpressed);
		if (isMine(qMakePair(row, col)))
		{
			mines_--;
		}
		return;
	}
	if (widget->getPress() == CustomButton::Pressing::pressed)
	{
		return;
	}
	widget->setPress(CustomButton::Pressing::flagged);
	setImage(widget, files["fileNameFlag"]);
	if (isMine(qMakePair(row, col)))
	{
		mines_++;
		isFinal();
	}
}
void Field::handleMidButtonClicked(int row, int col)
{
	if (static_cast< CustomButton * >(grid_->itemAtPosition(row, col)->widget())->getCntTypes() <= 0)
	{
		return;
	}
	int temp = static_cast< CustomButton * >(grid_->itemAtPosition(row, col)->widget())->getCntTypes();
	for (auto var : dxdy)
	{
		if (isNormal(qMakePair(row + var.first, col + var.second)) && isMine(qMakePair(row + var.first, col + var.second)) &&
			static_cast< CustomButton * >(grid_->itemAtPosition(row + var.first, col + var.second)->widget())->getPress() ==
				CustomButton::Pressing::flagged)
		{
			temp--;
		}
	}
	if (temp == 0)
	{
		for (auto var : dxdy)
		{
			if (isNormal(qMakePair(row + var.first, col + var.second)) && !isMine(qMakePair(row + var.first, col + var.second)))
			{
				walking(qMakePair(row + var.first, col + var.second));
			}
		}
	}
	else
	{
		for (auto var : dxdy)
		{
			if (!isNormal(qMakePair(row + var.first, col + var.second)))
			{
				continue;
			}
			CustomButton *but = static_cast< CustomButton * >(grid_->itemAtPosition(row + var.first, col + var.second)->widget());
			if (but->getPress() == CustomButton::Pressing::unpressed)
			{
				setImage(but, files["fileNameQuestion"]);

				QTimer::singleShot(300, [=]() { setImage(but, files["fileNameUnpress"]); });
			}
		}
	}
	isFinal();
}
Field::~Field()
{
	delete save_;
	delete restartButton_;
	delete setSave_;
	delete grid_;
	delete toolbar_;
	delete radioButton_;
	delete final_;
	delete gridWidget;
	delete topLayout;
	delete leastLaytout;
	delete centralWidget_;
}
