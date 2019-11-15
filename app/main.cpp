#ifndef Q_OS_UNIX
#include <SDL.h>
#endif

#include <QApplication>
#include <QStyleFactory>

#include "DarkStyle.h"
#include "gui/topwindow.h"


#ifdef Q_OS_UNIX
int main(int argc, char **argv)
#else
int SDL_main(int argc, char **argv)
#endif
{
#ifdef Q_OS_UNIX
	QCoreApplication::setAttribute( Qt::AA_X11InitThreads );
#endif
	
	QApplication app(argc, argv);
	//qDebug() << QStyleFactory::keys();
	app.setStyle(new DarkStyle);

	/*QTranslator translator;
	translator.load(QString("/home/cris/Devel/MachinTruc/machintruc_fr.qm"));
	app.installTranslator(&translator);*/
	
	srand((unsigned)time(0));

	TopWindow *tw = new TopWindow();
	tw->show();

	return app.exec();
}
