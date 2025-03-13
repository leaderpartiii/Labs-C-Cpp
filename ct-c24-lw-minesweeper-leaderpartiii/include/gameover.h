#ifndef GAMEOVER_H
#define GAMEOVER_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class GameOver : public QDialog
{
	Q_OBJECT

  public:
	explicit GameOver(QWidget *parent = 0);

	void templateFinal(QString file, QString message);

	~GameOver();

  private:
	QString winnigFile_;
	QString winningMessage_;
	QString losingFile_;
	QString losingMessage_;
	QLabel *imageLabel_;
	QLabel *messageLabel_;
	QPushButton *okButton_;
};

#endif	  // GAMEOVER_H
