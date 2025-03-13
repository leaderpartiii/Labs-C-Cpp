#include "include/dialogwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	QLabel *widthLabel = new QLabel("Width:", this);
	widthEdit_ = new QLineEdit(this);
	widthEdit_->setText("10");

	hbox1->addWidget(widthLabel);
	hbox1->addWidget(widthEdit_);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	QLabel *lengthLabel = new QLabel("Length:", this);
	lengthEdit_ = new QLineEdit(this);
	lengthEdit_->setText("10");
	hbox2->addWidget(lengthLabel);
	hbox2->addWidget(lengthEdit_);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	QLabel *minesLabel = new QLabel("Count of mines:", this);
	minesEdit_ = new QLineEdit(this);
	minesEdit_->setText("10");
	hbox3->addWidget(minesLabel);
	hbox3->addWidget(minesEdit_);

	QHBoxLayout *hboxButton = new QHBoxLayout();
	QPushButton *okButton = new QPushButton("OK", this);
	hboxButton->addStretch();
	hboxButton->addWidget(okButton);

	layout->addLayout(hbox1);
	layout->addLayout(hbox2);
	layout->addLayout(hbox3);
	layout->addLayout(hboxButton);

	connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
}

int Dialog::width() const
{
	return widthEdit_->text().toInt();
}

int Dialog::length() const
{
	return lengthEdit_->text().toInt();
}

int Dialog::mines() const
{
	return minesEdit_->text().toInt();
}
