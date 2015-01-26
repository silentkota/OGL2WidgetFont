#include "ogl2widget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	OGL2Widget w;
	w.show();
	return a.exec();
}
