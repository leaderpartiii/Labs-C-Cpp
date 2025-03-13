#include "include/custombutton.h"

#include <QApplication>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPair>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSet>
#include <QString>
#include <QVariant>

CustomButton::CustomButton(int row, int col, Types typing, QWidget *parent) :
	QPushButton(parent), m_row(row), m_col(col), typing_(typing)
{
}

void CustomButton::mousePressEvent(QMouseEvent *event)
{
	if (event->button() & Qt::RightButton)
	{
		emit rightButtonClicked(this->row(), this->col());
	}
	else if (event->button() & Qt::LeftButton)
	{
		emit leftButtonClicked(this->row(), this->col());
	}
	else if (event->button() & Qt::MidButton)
	{
		emit midButtonClicked(this->row(), this->col());
	}
	QWidget::mousePressEvent(event);
}
