#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <QDebug>
#include <QPushButton>

class CustomButton : public QPushButton
{
	Q_OBJECT

  public:
	enum Types
	{
		mines,
		nums,
		empty
	};
	enum Pressing
	{
		pressed,
		unpressed,
		flagged
	};

	CustomButton(int row, int col, Types type, QWidget *parent = 0);

	int row() const { return m_row; }
	int col() const { return m_col; }
	Types getType() const { return typing_; }
	void setType(Types typing) { typing_ = typing; }
	int getCntTypes() { return cntTypes_; }
	void setCntTypes(int cnt) { cntTypes_ = cnt; }
	Pressing getPress() { return isPress_; }
	void setPress(Pressing tr) { isPress_ = tr; }
	void mousePressEvent(QMouseEvent *event) override;
  public slots:
	void setRow(int row) { m_row = row; }
	void setCol(int col) { m_col = col; }
    void handleButtonClicked(){};

  signals:
	void leftButtonClicked(int x, int y);
	void rightButtonClicked(int x, int y);
	void midButtonClicked(int x, int y);

	void buttonClicked(Qt::MouseButton button);

  private:
	int m_row;
	int m_col;
	int cntTypes_ = 0;
	Types typing_;
	Pressing isPress_ = unpressed;
};

#endif	  // CUSTOMBUTTON_H
