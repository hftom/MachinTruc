#include <QApplication>
#include <QStyleFactory>

#include "DarkStyle.h"
#include "gui/topwindow.h"



int main(int argc, char **argv)
{
	QCoreApplication::setAttribute( Qt::AA_X11InitThreads );

	QApplication app(argc, argv);
	//qDebug() << QStyleFactory::keys();
	app.setStyle(new DarkStyle);

	/*QGLFormat glf = QGLFormat::defaultFormat();
	glf.setAlpha( true );
	glf.setSampleBuffers( true );
	glf.setSamples( 4 );
	QGLFormat::setDefaultFormat( glf );*/
	
	srand((unsigned)time(0));

	TopWindow *tw = new TopWindow();
	tw->show();

	return app.exec();
}
