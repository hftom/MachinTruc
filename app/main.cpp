#include <QApplication>

#include "gui/topwindow.h"



int main(int argc, char **argv)
{
	QCoreApplication::setAttribute( Qt::AA_X11InitThreads );

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
