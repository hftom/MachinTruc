#include <QApplication>

#include "gui/topwindow.h"



int main(int argc, char **argv)
{
#if !defined(Q_OS_MAC)
	QCoreApplication::setAttribute( Qt::AA_X11InitThreads );
#else
	QSurfaceFormat format;
	format.setVersion(3, 3);
	format.setProfile( QSurfaceFormat::CoreProfile );
	QSurfaceFormat::setDefaultFormat( format );
#endif

	QApplication app(argc, argv);

	TopWindow *tw = new TopWindow();
	tw->show();

	return app.exec();
}
