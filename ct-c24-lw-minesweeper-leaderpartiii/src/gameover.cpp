#include "include/gameover.h"

#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

GameOver::GameOver(QWidget *parent) : QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	imageLabel_ = new QLabel(this);
	layout->addWidget(imageLabel_);
	messageLabel_ = new QLabel(this);
	layout->addWidget(messageLabel_);
	okButton_ = new QPushButton("OK", this);
	connect(okButton_, &QPushButton::clicked, this, &GameOver::accept);
	layout->addWidget(okButton_);
	setLayout(layout);
}

void GameOver::templateFinal(QString file, QString message)
{
	imageLabel_->setPixmap(QPixmap(file));
	messageLabel_->setText(message);
}
GameOver::~GameOver()
{
	delete imageLabel_;
	delete messageLabel_;
	delete okButton_;
}
