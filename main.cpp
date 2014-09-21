#include <QApplication>

#include "MainDialog.h"

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath("plugins");
	QApplication app(argc, argv);
	MainDialog dlg;
	dlg.show();
	return app.exec();
}
