#include <QApplication>

#include "gui/topwindow.h"



extern "C" int XInitThreads();



int main(int argc, char **argv)
{
	XInitThreads();

	QApplication app(argc, argv);

	/*QGLFormat glf = QGLFormat::defaultFormat();
	glf.setAlpha( true );
	glf.setSampleBuffers( true );
	glf.setSamples( 4 );
	QGLFormat::setDefaultFormat( glf );*/

	TopWindow *tw = new TopWindow();
	tw->show();

	return app.exec();
}
