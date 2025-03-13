#include "include/dialogwindow.h"
#include "include/field.h"

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/images/images/mainIcon.png"));
	Dialog dialog;
    if (dialog.exec() == QDialog::Accepted)
    {}

	//	QStringList standardPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	QString configFilePath = /*standardPaths.at(0) + */ "/saving.ini";
	QSettings settings(configFilePath, QSettings::IniFormat);

	settings.beginGroup("GameState");

	int width = settings.value("width", dialog.width()).toInt();
	int length = settings.value("length", dialog.length()).toInt();
	int mines = settings.value("mines", dialog.mines()).toInt();
	bool ghostMode = settings.value("ghostMode", false).toBool();

	if (argc > 1 && QString(argv[1]) == "dbg")
		ghostMode = true;

	Field w(width, length, mines, ghostMode, &settings);
    w.show();

	return a.exec();
}
