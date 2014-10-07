#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QThread>
#include <QImage>
#include <QMutex>

#include "profile.h"

class Source;
class Frame;
class QGLWidget;



class ThumbResult
{
public:
	bool isValid;
	int inputType;
	QString path;
	Profile profile;
	QImage thumb;
	double thumbPTS;
};



class ThumbRequest
{
public:
	enum RequestType{ PROBE, THUMB };

	ThumbRequest( QString path )
		: typeOfRequest( PROBE ),
		filePath( path ),
		thumbPTS( 0 ) {}
		
	ThumbRequest( QString path, double pts )
		: typeOfRequest( THUMB ),
		filePath( path ),
		thumbPTS( pts ) {}
	
	int typeOfRequest;
	QString filePath;
	double thumbPTS;
	
	ThumbResult result;
};




class Thumbnailer : public QThread
{
	Q_OBJECT
public:
	Thumbnailer();
	void setSharedContext( QGLWidget *sharedContext );
	
	bool pushRequest( ThumbRequest req );
	
protected:
	void run();
	
private slots:
	void gotResult();
	
private:	
	//void probe( QStringList files, QList<Source*> *sources, QList<QImage> *thumbs );
	void probe( ThumbRequest &request );
	
	QImage getSourceThumb( Frame *f );
	
	QList<ThumbRequest> requestList;
	QList<ThumbRequest> resultList;
	
	QMutex requestMutex, resultMutex;
	bool running;
	QGLWidget *glContext;
	
signals:
	void resultReady();
	void thumbReady( ThumbResult );
};
#endif // THUMBNAILER_H
