#include "include/toolbar.h"

#include "include/field.h"

#include <QHBoxLayout>
#include <QIntValidator>

Toolbar::Toolbar(QWidget *parent) : QWidget(parent)
{
	toolBarLayout_ = new QHBoxLayout(this);

	widthEdit_ = new QLineEdit(this);
	widthEdit_->setPlaceholderText("Width");
	widthEdit_->setValidator(new QIntValidator(1, 100, this));
	toolBarLayout_->addWidget(widthEdit_);

	heightEdit_ = new QLineEdit(this);
	heightEdit_->setPlaceholderText("Height");
	heightEdit_->setValidator(new QIntValidator(1, 100, this));
	toolBarLayout_->addWidget(heightEdit_);

	minesEdit_ = new QLineEdit(this);
	minesEdit_->setPlaceholderText("Mines");
	minesEdit_->setValidator(new QIntValidator(1, 100, this));
	toolBarLayout_->addWidget(minesEdit_);

	applyButton_ = new QPushButton("Apply", this);
	toolBarLayout_->addWidget(applyButton_);

	setLayout(toolBarLayout_);

	connect(applyButton_, &QPushButton::clicked, this, &Toolbar::change);
}
int Toolbar::getHeight()
{
	return heightEdit_->text().toInt();
}
int Toolbar::getWidth()
{
	return widthEdit_->text().toInt();
}
int Toolbar::getMines()
{
	return minesEdit_->text().toInt();
}
void Toolbar::change()
{
	emit exchange(getHeight(), getWidth(), getMines());
}
Toolbar::~Toolbar()
{
    delete widthEdit_;
	delete heightEdit_;
	delete minesEdit_;
    delete applyButton_;
    delete toolBarLayout_;
}
